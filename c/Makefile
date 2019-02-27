out_name = utils.out
compiler = g++
std = c++11
source = utils.cpp
test_source = test/utils_test.cpp

all:
	# run test
	${compiler} ${source} ${test_source} -o ${out_name} --std=${std}
	echo "compile --- fin\n"
	./${out_name}

prod:
	${compiler} ${source} -o ${out_name} --std=${std} -Os

clear:
	rm ${out_name}

run:
	./${out_name}
