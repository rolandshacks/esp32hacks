@ECHO OFF
python ..\..\tools\render_boing.py
python ..\..\tools\bitmap2cpp.py -a -o bitmaps.inc ball0.png ball1.png ball2.png ball3.png ball4.png ball5.png
del ball0.png
del ball1.png
del ball2.png
del ball3.png
del ball4.png
del ball5.png
