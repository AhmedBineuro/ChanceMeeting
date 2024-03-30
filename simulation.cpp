// TO RUN JUST PRESS THE RUN BUTTON IN VSCODE
// OR RUN
// `g++ -std=c++20 simulation.cpp -lsfml-graphics -lsfml-system -lsfml-audio -lsfml-window -lGL -lpython3.10 -o ./exec/ChanceMeeting && ./exec/ChanceMeeting`
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include "SFML/Window.hpp"
#include "SFML/System.hpp"
#include "SFMLMath-master/src/SFMLMath.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include "python3.10/Python.h" //To run analysis script
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include "simParams.hpp"
/**
 * @brief This is the statistics simulation about how probable it is to meet your friends given that you are all travelling to different random cities
 */

/**
 * @brief An Agents is the moving entities meaning it contains the indices to the following:
 * - Position
 * - Velocity
 * - Wait timer
 * - City
 */

/**
 * @brief A City contains the indices of each of the four vertices that form the square.
 *  It also contains an array of agents
 */
struct City
{
    int topLeft; // For 4 vertices
    sf::Vector2f center;
    std::vector<int> agents;
};

// Simulation arrays
sf::Vector2f Positions[AGENT_COUNT];
sf::Vector2f Velocities[AGENT_COUNT];
int Destinations[AGENT_COUNT];
long long CitiesVisited[AGENT_COUNT];
long long GroupMet[AGENT_COUNT];
bool Waiting[AGENT_COUNT];

sf::Color GroupColors[GROUP_COUNT];

int Timers[AGENT_COUNT];
/// @brief Is set to true when an agent reaches their destination and is assigned a waiting time, also check for group members in city
long long VisitorCounter[CITY_COUNT]; // Counts how visitors a city gets
long long ChanceCounter[CITY_COUNT];  // Counts how many chance meetings occur inside a city
City Cities[CITY_COUNT];

sf::Time deltaTime, elapsed, sinceStart;
sf::Clock simClock;
sf::VertexArray CityVerts(sf::Quads);
sf::VertexArray AgentVerts(sf::Points);
sf::View view(sf::FloatRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));
float zoom = 1.0f;
sf::RenderWindow window;
sf::RenderTexture simPlane;
sf::Sprite simPlaneSprite;
sf::RectangleShape loadingBarBack, loadingBarFront;
bool running = false;

/// @brief DECLARATIONS
void initializeSimulation(int simSpaceWidth, int simSpaceHeight);
// Simulation
void updateSimulation();   // Function to step once and update the simulation
void updateViewPosition(); // Function to update view location
void drawSimulation();     // Function to draw the simulation

// Helper
void checkForGroup(int index); // A group is defined by the group index
///@brief Dumps the simulation statistics into the terminal
void DumpResults();
///@brief Saves the simulation statistics to a bunch of csv files
void SaveResults();
/// @brief Runs python script to generate plots for the data we gathered;
void RunAnalysis(); // Runs analysis.py python script

int main()
{
    initializeSimulation(SIM_PLANE_WIDTH, SIM_PLANE_HEIGHT);

    // Loading bar segment
    loadingBarBack.setSize(sf::Vector2f(static_cast<float>(LOADING_BAR_WIDTH), static_cast<float>(LOADING_BAR_HEIGHT)));
    loadingBarFront.setSize(sf::Vector2f(0, static_cast<float>(LOADING_BAR_HEIGHT)));
    loadingBarBack.setFillColor(sf::Color(100, 0, 100));
    loadingBarFront.setFillColor(sf::Color(165, 0, 165));
    loadingBarBack.setPosition((static_cast<float>(WINDOW_WIDTH) / 2.0f) - (static_cast<float>(LOADING_BAR_WIDTH) / 2.0f), static_cast<float>(WINDOW_HEIGHT) - static_cast<float>(LOADING_BAR_HEIGHT) * 2.5f);
    loadingBarFront.setPosition((static_cast<float>(WINDOW_WIDTH) / 2.0f) - (static_cast<float>(LOADING_BAR_WIDTH) / 2.0f), static_cast<float>(WINDOW_HEIGHT) - static_cast<float>(LOADING_BAR_HEIGHT) * 2.5f);

    window.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "CHANCE MEETING");
    simPlane.create(SIM_PLANE_WIDTH, SIM_PLANE_HEIGHT);
    simPlane.setActive(true);
    simPlaneSprite.setTexture(simPlane.getTexture());
    simPlaneSprite.setOrigin(static_cast<float>(SIM_PLANE_WIDTH) / 2.0f, static_cast<float>(SIM_PLANE_HEIGHT) / 2.0f);
    simPlaneSprite.setPosition(static_cast<float>(WINDOW_WIDTH) / 2.0f, static_cast<float>(WINDOW_HEIGHT) / 2.0f);
    glPointSize(AGENT_SIZE);
    view.setCenter(SIM_PLANE_WIDTH / 2, SIM_PLANE_HEIGHT / 2);
    deltaTime = simClock.restart();
    elapsed += simClock.getElapsedTime();
    running = window.isOpen();
    while (running)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                running = false;
            }
            // Detect scroll up and down for the zoom
            if (event.type == sf::Event::MouseWheelScrolled)
            {
                float zoomFactor = (event.mouseWheelScroll.delta > 0) ? 0.9f : 1.1f;
                view.zoom(zoomFactor);
                simPlane.setView(view);
            }
        }
        elapsed += simClock.getElapsedTime();
        sinceStart += simClock.getElapsedTime();
        deltaTime = simClock.restart();
        drawSimulation();
        while (elapsed >= deltaTime)
        {
            updateViewPosition();
            updateSimulation();
            elapsed -= deltaTime;
        }
        if (sinceStart >= sf::seconds(SIMULATION_EXECUTION_TIME))
            running = false;
        else
        {
            sf::Vector2f newDimensions;
            newDimensions.x = static_cast<float>(sinceStart.asSeconds()) / static_cast<float>(SIMULATION_EXECUTION_TIME) * static_cast<float>(LOADING_BAR_WIDTH);
            newDimensions.y = static_cast<float>(LOADING_BAR_HEIGHT);
            loadingBarFront.setSize(newDimensions);
        }
    }
    window.close();
    SaveResults();
    RunAnalysis();
    return 0;
}

void initializeSimulation(int simSpaceWidth, int simSpaceHeight)
{
    deltaTime = sf::seconds(1.0f / 60.0f);
    srand(time(NULL));
    for (int i = 0; i < GROUP_COUNT; i++)
    {
        float r = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (255.0f);
        float g = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (255.0f);
        float b = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (255.0f);
        GroupColors[i] = sf::Color(r, g, b);
    }

    for (int i = 0; i < CITY_COUNT; i++)
    {
        VisitorCounter[i] = 0;
        float left = CITY_WIDTH + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * static_cast<float>(simSpaceWidth - CITY_WIDTH) + 1;
        float top = CITY_WIDTH + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * static_cast<float>(simSpaceHeight - CITY_WIDTH) + 1;
        float r = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 255;
        float g = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 150;
        float b = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 150;
        sf::Vertex v;
        v.color = sf::Color(r, g, b, 160);
        v.position = sf::Vector2f(left, top);
        CityVerts.append(v);
        Cities[i].topLeft = CityVerts.getVertexCount() - 1;
        Cities[i].center = sf::Vector2f(v.position.x + (CITY_WIDTH / 2), v.position.y + (CITY_WIDTH / 2));

        v.position = sf::Vector2f(left + CITY_WIDTH, top);
        CityVerts.append(v);

        v.position = sf::Vector2f(left + CITY_WIDTH, top + CITY_WIDTH);
        CityVerts.append(v);

        v.position = sf::Vector2f(left, top + CITY_WIDTH);
        CityVerts.append(v);
        Cities[i].agents.clear();
    }
    for (int i = 0; i < AGENT_COUNT; i++)
    {
        sf::Vertex v;
        float x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * static_cast<float>(simSpaceWidth) + 1;
        float y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * static_cast<float>(simSpaceHeight) + 1;
        Positions[i] = sf::Vector2f(x, y);
        v.position = Positions[i];
        v.color = GroupColors[GROUP_INDEX(i)];
        AgentVerts.append(v);
        // Pick a City to start heading towards
        Timers[i] = 0;
        Waiting[i] = false;
        Destinations[i] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * static_cast<float>(CITY_COUNT);
        CitiesVisited[i] = 0;
        GroupMet[i] = 0;
    }
    view.zoom(20.0f);
}
void updateSimulation()
{
    for (int i = 0; i < AGENT_COUNT; i++)
    {
        if (!Waiting[i])
        {

            if (sf::getLength(Cities[Destinations[i]].center - Positions[i]) >= AGENT_SIZE)
            {
                Velocities[i] = sf::getNormalized(Cities[Destinations[i]].center - Positions[i]) * MAX_VELOCITY;
                Positions[i] += Velocities[i];
                AgentVerts[i].position = Positions[i];
            }
            else
            {
                CitiesVisited[i]++;
                VisitorCounter[Destinations[i]]++;
                checkForGroup(i);
                Cities[Destinations[i]].agents.push_back(i);
                Waiting[i] = true;
                Timers[i] = MIN_WAIT_TIME + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (MAX_WAIT_TIME - MIN_WAIT_TIME);
            }
        }
        else
        {
            if (Timers[i] > 0)
            {
                Timers[i]--;
            }
            else
            {
                Waiting[i] = false;
                int currentCity = Destinations[i];
                int newCity = -1;
                do
                {
                    newCity = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * static_cast<float>(CITY_COUNT);
                } while (newCity == currentCity);
                auto it = std::find(Cities[Destinations[i]].agents.begin(), Cities[Destinations[i]].agents.end(), i);
                if (it != Cities[Destinations[i]].agents.end())
                    Cities[Destinations[i]].agents.erase(it);
                Destinations[i] = newCity;
            }
        }
    }
}
void updateViewPosition()
{
    bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        if (!shift)
            view.move(0, -MAX_VELOCITY);
        else
            view.move(0, -8 * MAX_VELOCITY);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        if (!shift)
            view.move(0, MAX_VELOCITY);
        else
            view.move(0, 8 * MAX_VELOCITY);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        if (!shift)
            view.move(-MAX_VELOCITY, 0);
        else
            view.move(-8 * MAX_VELOCITY, 0);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        if (!shift)
            view.move(MAX_VELOCITY, 0);
        else
            view.move(8 * MAX_VELOCITY, 0);
    }
    // window.setView(view);
    simPlane.setView(view);
}
void drawSimulation()
{

    simPlane.setActive(true);
    simPlane.clear(sf::Color::Transparent);
    simPlane.draw(CityVerts);
    simPlane.draw(AgentVerts);
    simPlane.setView(view);
    simPlane.display();

    window.setActive(true);
    window.clear(sf::Color::White);
    window.draw(simPlaneSprite);
    window.draw(loadingBarBack);
    window.draw(loadingBarFront);
    window.display();
}
void checkForGroup(int index)
{
    for (int agent_index : Cities[Destinations[index]].agents)
    {
        if (GROUP_INDEX(index) == GROUP_INDEX(agent_index))
        {
            GroupMet[index]++;
            ChanceCounter[Destinations[index]]++;
        }
    }
}

void DumpResults()
{

    std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
    std::cout << "Agent Index" << ',' << "Cities Visited" << ',' << "Chance meetings with group" << std::endl;
    for (int i = 0; i < AGENT_COUNT; i++)
    {
        std::cout << i << ',' << CitiesVisited[i] << ',' << GroupMet[i] << std::endl;
    }
    std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;

    std::cout << std::endl;

    std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
    std::cout << "Group Index" << ',' << "Average Meets" << ',' << "Total" << std::endl;
    for (int j = 0; j < GROUP_COUNT; j++)
    {
        int agentCount = 0;
        int totalMeetings = 0;
        for (int i = j; i < AGENT_COUNT; i += GROUP_COUNT)
        {
            agentCount++;
            totalMeetings += GroupMet[i];
        }
        float average = static_cast<float>(totalMeetings) / static_cast<float>(agentCount);
        std::cout << j << ',' << average << ',' << totalMeetings << std::endl;
    }
    std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;

    std::cout << std::endl;

    std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
    std::cout << "City Index" << ',' << "Visitor Count" << ',' << "Chance Meeting Count" << std::endl;
    for (int i = 0; i < CITY_COUNT; i++)
    {
        std::cout << i << ',' << VisitorCounter[i] << ',' << ChanceCounter[i] << std::endl;
    }
    std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
}

void SaveResults()
{
    std::ofstream file;
    file.open(("./SimStats/AgentData.csv"));
    if (file.is_open())
    {
        file << "Agent Index,Group Index,Cities Visited,Chance meetings with group\n";
        for (int i = 0; i < AGENT_COUNT; i++)
        {
            file << i << ',' << i % (GROUP_COUNT) << ',' << CitiesVisited[i] << ',' << GroupMet[i] << '\n';
        }
        file.close();
    }

    file.open("./SimStats/GroupData.csv");
    if (file.is_open())
    {
        file << "Group Index,Average Meets,Total\n";
        for (int j = 0; j < GROUP_COUNT; j++)
        {
            int agentCount = 0;
            int totalMeetings = 0;
            for (int i = j; i < AGENT_COUNT; i += GROUP_COUNT)
            {
                agentCount++;
                totalMeetings += GroupMet[i];
            }
            float average = static_cast<float>(totalMeetings) / static_cast<float>(agentCount);
            file << j << ',' << average << ',' << totalMeetings << '\n';
        }
        file.close();
    }

    file.open("./SimStats/CityData.csv");
    if (file.is_open())
    {
        file << "City Index,Average Meets,Visitor Count,Total Meets\n";
        for (int i = 0; i < CITY_COUNT; i++)
        {
            float average = static_cast<float>(VisitorCounter[i]) / static_cast<float>(ChanceCounter[i]);
            if (isinff(average) || isnanf(average))
                average = 0;
            file << i << ',' << average << ',' << ChanceCounter[i] << ',' << VisitorCounter[i] << '\n';
        }
        file.close();
    }
}

void RunAnalysis()
{
    Py_Initialize();
    FILE *file = fopen("./analysis.py", "r");
    if (file != NULL)
    {
        PyRun_SimpleFile(file, "./analysis.py");
        fclose(file);
    }
    Py_Finalize();
}