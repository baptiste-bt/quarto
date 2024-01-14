# Quarto
An implementation of the Quarto game for Reinforcement Learning, with code to train and test several different agents (PPO, A2C, MaskablePPO).


### Motivation
This project was completed in order to learn the basics of RL to play a simple game such as Quarto.

### Built with
- numpy
- stable-baselines3
- sb3_contrib

### Train instructions
```
python train.py --steps [N_STEPS] --algo [ALGO] [--path [PATH_TO_MODEL]]

```
To train using MaskablePPO, run:
```
python train_masked.py --steps [N_STEPS] --algo ppo_mask [--path [PATH_TO_MODEL]]

```

### Test instructions
```
python test_model.py --algo [ALGO] --path [PATH_TO_MODEL] --episodes [N_EPISODES]
```

# Quarto & MinMax with alpha beta pruning

| Algorithm         | Depth 1           | Depth 2           | Depth 3           |
|-------------------|-------------------|-------------------|-------------------|
|                   | Time (s) | Nodes     | Time (s) | Nodes     | Time (s) | Nodes     |
|-------------------|---------|-----------|---------|-----------|---------|-----------|
| minMax            | <0.01   | 256       | 0.02    | 57600     | 2.35    | 11289600 |
| Alpha-beta pruning| <0.01   | 256       | 0.02    | 57600     | 0.07    | 107520   |

