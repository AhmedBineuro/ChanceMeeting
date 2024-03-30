#define SIMULATION_EXECUTION_TIME 35.0f * 60.0f // In Seconds

#define LOADING_BAR_WIDTH 800
#define LOADING_BAR_HEIGHT 20

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000

///@brief The simulation plane is where the simulation will happen and where it will be drawn to
#define SIM_PLANE_WIDTH 8000
#define SIM_PLANE_HEIGHT 8000

// Agent's waiting time at a city
#define MIN_WAIT_TIME 500   // In simulation ticks
#define MAX_WAIT_TIME 60000 // In simulation ticks
// Agent's velocity
#define MAX_VELOCITY 4.0f

// Array Sizes
#define AGENT_COUNT 20000
#define CITY_COUNT 1000
#define GROUP_COUNT 10000

// Drawing dimensions
#define CITY_WIDTH 100.0f
#define AGENT_SIZE 4.0f
#define GROUP_INDEX(AGENT_INDEX) (AGENT_INDEX % GROUP_COUNT)