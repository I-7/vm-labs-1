all:
	i=1; while [[ $$i -le 10 ]] ; do \
        echo $$((2 ** i)) ; \
        ((i = i + 1)) ; \
    done
	g++ -DARRAY_SIZE=33 -pthread -o main main.cpp
	./main
