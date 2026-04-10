all: julia

ifeq ($(UNAME), Darwin)
	LDFLAGS = -L/opt/homebrew/opt/libomp/lib -lomp
endif

ffmpeg:
	if test ! -e ffmpeg -a ! -h ffmpeg; then if which ffmpeg > /dev/null; then ln -s $$(which ffmpeg) ./ffmpeg; else curl https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz | tar xJ --absolute-names --no-anchored --transform='s:.*/::' ffmpeg-6.1-amd64-static/ffmpeg; fi; fi

src/frame.o: src/frame.cc src/include/frame.h src/include/consts.h
	g++ -O2 -fopenmp $(LDFLAGS) -I./src/include -c -o src/frame.o src/frame.cc

src/animation.o: src/animation.cc src/include/animation.h src/include/consts.h
	g++ -O2 -fopenmp $(LDFLAGS) -I./src/include -c -o src/animation.o src/animation.cc

julia: main.cc src/frame.o src/animation.o
	mpic++ -O2 -fopenmp $(LDFLAGS) -I./src/include -o julia main.cc src/frame.o src/animation.o --std=c++23

run: ffmpeg julia
	mpirun --bind-to none -np $(or $(NP),4) ./julia

help:
	@echo "Use \`make\` to build and \`make run\` to execute on 4 processes (default). Use \`make run NP=2\` to choose the number of processes."

clean:
	rm -f src/frame.o src/animation.o julia
