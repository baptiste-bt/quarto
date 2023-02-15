"""Train a Reinforcement Learning agent. Avalaible agents are PPO, A2C and MaskedPPO
To run:
    python train_masked.py --steps [N_STEPS] -algo [ALGO]

    or:
    python train_masked.py --steps [N_STEPS] -algo [ALGO] --path [PATH_TO_MODEL]

    to resume training.

"""


from stable_baselines3.ppo import PPO
from stable_baselines3.a2c import A2C
from stable_baselines3.common.vec_env import SubprocVecEnv
from stable_baselines3.common.monitor import Monitor
from sb3_contrib.ppo_mask import MaskablePPO
from quarto_env import QuartoEnv
import torch as th
import argparse

N_ENVS = 8

def main():
    parser = argparse.ArgumentParser(description='Lauching training...')
    parser.add_argument("--steps", type=int, help="number of training steps", required=True)
    parser.add_argument("--path", help="path of model to continue training")
    parser.add_argument('-a', "--algo", help="RL algo to use available: a2c, ppo, sac, td3", required=True)

    args = parser.parse_args()

    env = SubprocVecEnv([lambda: Monitor(QuartoEnv(opponent='random'), './logs') for k in range(N_ENVS)])

    model = get_model(args.path, env, args.algo)

    model.learn(total_timesteps=args.steps, tb_log_name=f"{args.algo}_{args.steps}_4x64")

    model.save(f'./models/{args.algo}_{args.steps}')


def get_model(path, env, algo):
    if algo == 'ppo':   
        if path is None:
            # policy_kwargs = dict(activation_fn=th.nn.ReLU,
            #                 net_arch=dict(pi=[64, 64, 64, 64], vf=[64, 64]))
            model = PPO("MlpPolicy", env, verbose=1,
                        n_steps=256,
                        tensorboard_log='./logs',
                        # policy_kwargs=policy_kwargs
                        )
        else:
            model = PPO.load(path, env)
    elif algo == 'a2c':
        if path is None:
            # policy_kwargs = dict(activation_fn=th.nn.ReLU,
            #                 net_arch=dict(pi=[64, 64, 64, 64], vf=[64, 64]))
            model = A2C("MlpPolicy", env, verbose=1,
                        tensorboard_log='./logs',
                        )
        else:
            model = A2C.load(path, env)
    elif algo == 'ppo_mask':
        if path is None:
            model = MaskablePPO("MlpPolicy", env, verbose=1,
                                # n_steps=256,
                                tensorboard_log='./logs')


    return model


if __name__ == '__main__':
    main()