"""Test a Reinforcement Learning agent against a random player at Quarto. Avalaible agents are PPO, A2C and MaskedPPO
To run:
    python test_model.py --algo [ALGO] --path [PATH_TO_MODEL] --episodes [N_EPISODES]

"""


from stable_baselines3.ppo import PPO
from stable_baselines3.a2c import A2C
from sb3_contrib.ppo_mask import MaskablePPO
from quarto_env import QuartoEnv
import numpy as np
import argparse

def main():
    parser = argparse.ArgumentParser(description='Lauching training...')
    parser.add_argument("--path", help="path of model to continue training", required=True)
    parser.add_argument("-e", "--episodes", type=int, help="number of testing episodes", default=100)
    parser.add_argument('-a', "--algo", help="RL algo to use available: a2c, ppo", required=True)
    args = parser.parse_args()

    env = QuartoEnv(opponent='random')
    model = load_model(args.algo, args.path, env)
    step = 0
    invalid_actions = 0
    wins = 0
    rewards = np.zeros(args.episodes)

    for episode in range(args.episodes):
        obs = env.reset()
        done = False
        
        while not done:
            # env.render()
            if args.algo == "ppo_mask":
                action_masks = env.action_masks()
                action, _ = model.predict(obs, action_masks=action_masks)
            else:
                action, _ = model.predict(obs)
            # _ = input('Press enter...')
            # print('action:', action)
            # action = env.action_space.sample()
            obs, reward, done, info = env.step(action)
            rewards[episode] += reward
            if reward == -1:
                invalid_actions += 1
            if reward == 100:
                wins += 1
            step += 1
        print(f'Episode {episode}, reward: {rewards[episode]}')

    print(f'\nMean reward after {args.episodes} ({step} steps): {rewards.mean()}, invalid actions: {invalid_actions/step:.3f}, percent win: {wins/args.episodes}')

def load_model(algo, path, env):
    if algo == 'ppo':
        cl = PPO
    elif algo == 'a2c':
        cl = A2C
    elif algo == 'ppo_mask':
        cl = MaskablePPO

    return cl.load(path, env)

if __name__ == '__main__':
    main()