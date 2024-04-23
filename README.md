# OS_Airport_management



gcc plane.c -g -lrt -fno-stack-protector -o plane -pthread
gcc airtrafficcontroller.c -g -lrt -fno-stack-protector -o airtrafficcontroller -pthread
gcc airport.c -g -lrt -fno-stack-protector -o airport -pthread
gcc cleanup.c -g -lrt -fno-stack-protector -o cleanup -pthread

/*
For plane to ATC
currentMessage.sender = 1;
currentMessage.r = plane_data;

For ATC to plane
currentMessage.sender = 2;
currentMessage.CONFORMATION_plane = CONFORMATION_plane;

For ATC to airport for departure
currentMessage.sender = 2;
currentMessage.FOR_DEPARTURE = 1;
currentMessage.r = r;

For airport to ATC (When boarding: DEPARTURE_case = 1)
currentMessage.sender = 3;
currentMessage.FOR_DEPARTURE = 1;
currentMessage.DEPARTURE_case = 1;
currentMessage.CONFORMATION_boarding = CONFORMATION_boarding;
currentMessage.r = r;

For airport to ATC (When takeoff: DEPARTURE_case = 2)
currentMessage.sender = 3;
currentMessage.FOR_DEPARTURE = 1;
currentMessage.DEPARTURE_case = 2;
currentMessage.CONFORMATION_takeoff = CONFORMATION_takeoff;
currentMessage.r = r;

For ATC to airport for arrival
currentMessage.sender = 2;
currentMessage.FOR_DEPARTURE = 0;
currentMessage.r = r;


For airport to ATC (When landing: ARRIVAL_case = 1)
currentMessage.sender = 3;
currentMessage.FOR_DEPARTURE = 0;
currentMessage.ARRIVAL_case = 1;
currentMessage.CONFORMATION_landing = CONFORMATION_landing;
currentMessage.r = r;

For airport to ATC (When deboarded: ARRIVAL_case = 2)
currentMessage.sender = 3;
currentMessage.FOR_DEPARTURE = 0;
currentMessage.ARRIVAL_case = 2;
currentMessage.CONFORMATION_deboarded = CONFORMATION_deboarded;
currentMessage.r = r;

For ATC to airport
currentMessage.sender = 2;
currentMessage.TERMINATION = TERMINATION;

For cleanup to ATC
currentMessage.sender = 4;
currentMessage.TERMINATION = 1;




*/
