import pandas as pd
import matplotlib.pyplot as plt
DATA_DIR = "./SimStats/"
PLOT_DIR = "./plots/"

AGENT_FILE_NAME = "AgentData.csv"
GROUP_FILE_NAME = "GroupData.csv"
CITY_FILE_NAME = "CityData.csv"

if __name__ == "__main__":
    print("\nStarting Analysis\n")
    path = DATA_DIR+AGENT_FILE_NAME
    AgentDF = pd.read_csv(path)
    path = DATA_DIR+GROUP_FILE_NAME
    GroupDF = pd.read_csv(path)
    path = DATA_DIR+CITY_FILE_NAME
    CityDF = pd.read_csv(path)
    plt.close('all')
    groupChancePair = AgentDF[["Group Index", "Chance meetings with group"]]
    citiesChancePair = AgentDF[[
        "Cities Visited", "Chance meetings with group"]]
    # Agent Data
    ordered = groupChancePair.groupby(
        "Group Index")["Chance meetings with group"].mean().plot(kind='bar', fontsize=5, figsize=(10, 6), rot=90)
    plt.xlabel("Group Index")
    plt.ylabel("Chance Meetings")
    plt.savefig(PLOT_DIR+'groupChancePair.png')
    plt.close()
    citiesChancePair.plot(kind='scatter', x='Cities Visited',
                          y='Chance meetings with group', fontsize=14, figsize=(10, 6), rot=90)
    plt.xlabel("Cities Visited")
    plt.ylabel("Chance Meetings")
    plt.savefig(PLOT_DIR+'citiesChancePair.png')
    plt.close()
    # City Data
    # "City Index,Average Meets,Visitor Count,Total Meets"
    cityVisitPair = CityDF[["City Index", "Visitor Count"]]
    visitChancePair = CityDF[["City Index", "Total Meets"]]
    cityVisitPair.plot(kind='bar', x='City Index',
                       y='Visitor Count', fontsize=5, figsize=(10, 6), rot=90)
    plt.xlabel("City Index")
    plt.ylabel("Visits")
    plt.savefig(PLOT_DIR+'cityVisitPair.png')
    plt.close()
    visitChancePair.plot(kind='bar', x='City Index',
                         y='Total Meets', fontsize=5, figsize=(10, 6), rot=90)
    plt.xlabel("City Index")
    plt.ylabel("Total Chance Meetings")
    plt.savefig(PLOT_DIR+'cityChancePair.png')
    plt.close()
    print("\nAnalysis Complete\n")
