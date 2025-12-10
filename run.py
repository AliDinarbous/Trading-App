import subprocess
import time

def lancer_terminal(cmd, titre="Processus"):
    subprocess.Popen([
        "gnome-terminal", "--", "bash", "-c", f'echo {titre}; {cmd}; exec bash'
    ])

# Lancer le Market
market_cmd = "./build/src/market/market -p 3000 -n 3 -f data/market/cac40.csv"
print("Lancement du market...")
lancer_terminal(market_cmd, "Market")

# Pause pour s'assurer que le market démarre
time.sleep(2)

# Lancer les agents
for i in range(1, 4):
    port = 4000 + i
    agent_cmd = f"./build/src/agent/agent -p {port} -n 3 -i {i} -m 3000 -d data/agent/agent_{i}.json"
    print(f"Lancement de l'agent {i} sur le port {port}...")
    lancer_terminal(agent_cmd, f"Agent {i}")
