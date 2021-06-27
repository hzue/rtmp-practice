# rtmp-practice

  * Just get a picture of rtmp things
  * All resources are from this [course](https://www.udemy.com/course/cffmpeg-ffmpegqt5opencv/)
  * Makefile, shared lib in this repo are for MacOS

## crtmpserver

  * build & run rtmp server

      ```bash
      $ make build-crtmpserver
      $ make run-crtmpserver
      ```
 
  * push stream

      ```
      $ ffmpeg -re -i test.flv -c copy -f flv rtmp://127.0.0.1/live/test
      ```

## nginx-rtmp

  * build & run rtmp server

      ```bash
      $ make build-nginx_rtmp
      $ make run-nginx_rtmp
      ```

  * push stream

      ```
      $ ffmpeg -re -i test.flv -c copy -f flv rtmp://127.0.0.1/live
      ```

## Client (pull streaming)

  1. VLC
  1. ffplay

      ```
      ffplay rtmp://127.0.0.1/live -fflags nobuffer
      ```
