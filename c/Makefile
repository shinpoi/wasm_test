out_name = utils.out
std = c99

all:
	gcc utils.c -o ${out_name} --std=${std}

prod:
	gcc utils.c -o ${out_name} --std=${std} -Os

clear:
	rm ${out_name}

run:
	./${out_name}
