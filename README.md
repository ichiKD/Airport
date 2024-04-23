# OS_Airport_management



gcc plane.c -g -lrt -fno-stack-protector -o plane -pthread
gcc airtrafficcontroller.c -g -lrt -fno-stack-protector -o airtrafficcontroller -pthread
gcc airport.c -g -lrt -fno-stack-protector -o airport -pthread
gcc cleanup.c -g -lrt -fno-stack-protector -o cleanup -pthread

