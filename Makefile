all: gol_win32.exe

gol_win32.exe: resource.res
	cl.exe /O2 src\gameoflife.cc /link kernel32.lib shell32.lib user32.lib gdi32.lib src\resource.res

resource.res:
	rc.exe src\resource.rc

