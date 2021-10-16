all: build

build: so-multi.obj
	cl /MD /W3 so-multi.obj /Feso-cpp.exe

so-multi.obj: so-multi.c
	cl /MD /W3 so-multi.c /c /Foso-multi.obj
clean:
	del so-cpp.exe so-multi.obj
