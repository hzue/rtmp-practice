extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/time.h"
}

#include <iostream>
using namespace std;

int av_error(int error_code)
{
    char buf[1024] = {0};
    av_strerror(error_code, buf, sizeof(buf));
    cout << "AV error: " << buf << endl;
    return -1;
}

static double r2d(AVRational r)
{
    return r.num  == 0 || r.den == 0 ?  0. : (double)r.num / (double)r.den;
}

int main(int argc, char *argv[]) {
    // Initialize networking
    avformat_network_init();

    /*
     * Input streaming
     */
    AVFormatContext *ictx = NULL;
    char inURL[] = "./vedios/test.flv";
    int re = avformat_open_input(&ictx, inURL, 0, 0);
    if (re != 0) return av_error(re);

    cout << "Open file " << inURL << " successfully." << endl;

    // get stream information of video, audio (h264, flv)
    re = avformat_find_stream_info(ictx, 0);
    if (re) return av_error(re);

    cout << endl;
    av_dump_format(ictx, 0, inURL, 0);

    /*
     * Output streaming
     */
    AVFormatContext *octx = NULL;
    char outURL[] = "rtmp://127.0.0.1/live";
    re = avformat_alloc_output_context2(&octx, 0, "flv", outURL);
    if (re != 0) return av_error(re);

    for (int i = 0; i < ictx->nb_streams; i++)
    {
        AVStream *out = avformat_new_stream(octx, ictx->streams[i]->codec->codec);
        if (!out) return av_error(0);
        re = avcodec_parameters_copy(out->codecpar, ictx->streams[i]->codecpar);
        if (re != 0) return av_error(re);
        out->codec->codec_tag = 0;
    }

    cout << endl;
    av_dump_format(octx, 0, outURL, 1);

    /*
     * rtmp push streaming
     */
    re = avio_open(&octx->pb, outURL, AVIO_FLAG_WRITE);
    if (!octx->pb) return av_error(re);

    re = avformat_write_header(octx, 0);
    if (re < 0) return av_error(re);

    // push every frame
    AVPacket pkt;
    long long start_time = av_gettime();
    for (;;)
    {
        re = av_read_frame(ictx, &pkt);
        if (re != 0) break;
        cout << pkt.pts << " " << flush;

        // caculate timestamp (pts/dts)
        AVRational itime = ictx->streams[pkt.stream_index]->time_base;
        AVRational otime = octx->streams[pkt.stream_index]->time_base;

        pkt.pts = av_rescale_q_rnd(pkt.pts, itime, otime, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.pts, itime, otime, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q_rnd(pkt.duration, itime, otime, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.pos = -1; // there is no position for streaming

        if (ictx->streams[pkt.stream_index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            AVRational tb = ictx->streams[pkt.stream_index]->time_base;
            long long now = av_gettime() - start_time;
            long long dts = 0;
            dts = pkt.dts * (1000 * 1000 * r2d(tb));
            if (dts > now) av_usleep(dts - now);
        }

        re = av_interleaved_write_frame(octx, &pkt);
        if (re < 0) return av_error(re);
        // av_packet_unref(&pkt);
    }

    return 0;
}
