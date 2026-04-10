all: julia

ifeq ($(UNAME), Darwin)
	LDFLAGS = -L/opt/homebrew/opt/libomp/lib -lomp
endif

ffmpeg:
	if test ! -e ffmpeg -a ! -h ffmpeg; then if which ffmpeg > /dev/null; then ln -s $$(which ffmpeg) ./ffmpeg; else curl https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz | tar xJ --absolute-names --no-anchored --transform='s:.*/::' ffmpeg-6.1-amd64-static/ffmpeg; fi; fi

%.o: %.cc %.h consts.h
	g++ -O2 $(LDFLAGS) -c -o $@ $<

julia: main.cc frame.o animation.o consts.h
	mpic++ -O2 $(LDFLAGS) -o julia main.cc frame.o animation.o --std=c++23

run: ffmpeg julia
	mpirun -np $(or $(NP),4) ./julia

help:
	@echo "Use \`make\` to build and \`make run\` to execute on 4 processes (default). Use \`make run NP=2\` to choose the number of processes."

clean:
	rm *.o julia
