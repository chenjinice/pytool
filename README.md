
pytool

win10生成单个可执行文件:
	pyinstaller -F main.py     ,  在dist目录 main.exe


运行:
	1.把config,asn和web目录放到dist目录,或者把main.exe拷出来
	2.然后点击main.exe运行
	（所以不提前放进去，是为了编码时方便调试）