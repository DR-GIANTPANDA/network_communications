src=$(wildcard *.cpp)

obj=$(patsubst %.cpp, %.o, $(src))

target=app

shared=-lpthread -ljsoncpp -lmysqlclient

$(target):$(obj)
	g++ $(obj) $(shared) -o $(target)

%.o:%.cpp
	g++ $< -c

.PHONY:clean 
clean:
	rm $(obj) $(target)
