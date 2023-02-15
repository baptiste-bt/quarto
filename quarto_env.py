"""Quarto environment for Reinforcement Learning"""

import random
import numpy as np
import gym

MAX_LEN_EPISODE = 50

class QuartoEnv(gym.Env):
    
    def __init__(self, opponent=None):

        self.action_space = gym.spaces.MultiDiscrete(np.array([16, 16]))
        self.observation_space = gym.spaces.MultiBinary([4, 4, 6]) #5 to have an extra layer to specify if there is a piece at this place
        self.pieces = np.ones(16, dtype=np.uint8)
        self.board = np.zeros([4, 4, 6], dtype=np.uint8) ###State corresponding to board and piece
        self.valid_move = True
        self.done = False
        self.score = None
        self._opponent = opponent
        self.n_turns = 0
        
    def step(self, action):
        assert action[0] >= 0 and action[0] < 16, "The place should be in [0, 15]"
        assert action[1] >= 0 and action[1] < 16, "The piece given should be in [0, 15]"

        place, next_piece = action
        self.n_turns += 1
        
        i, j = self._int_to_coord(place)
        if self.board[i, j, 4] == 1:
            self.valid_move = False
        
        elif self.pieces[next_piece] == 0 and self.pieces.sum() != 0:
            self.valid_move = False
        
        else:
            self._put_piece_on_board(i, j)

            self._add_next_piece_to_state(next_piece)  

            # Pb with win on last piece -> solved ?
            self.done, self.score = self._check_win(i, j)
        
            if self._opponent and not self.done:
                self._opponent_turn()
        
        assert self.board[:, :, 5].sum() == 1

        reward = self._get_reward()
        return self.board, reward, self.done, {} # Last item {} is necessary but useless here
    
    def reset(self):
        self.board = np.zeros([4, 4, 6], dtype=np.uint8)
        self.pieces = np.ones(16, dtype=np.uint8)
        self.valid_move = True
        self.done = False
        self.score = None
        self.n_turns = 0

        # We choose the first piece at random
        first_piece = random.randint(0, 15)
        self.pieces[first_piece] = 0
        self.board[first_piece//4, first_piece%4, 5] = 1
        return self.board
    
    def _put_piece_on_board(self, i, j):
        assert self.board[i, j, 4] == 0
        piece = np.argmax(self.board[:, :, -1])
        self.board[i, j, 4] = 1 #Put next_piece on board
        self.board[i, j, :4] = self._piece_to_binary(piece) #Plug characteristics in board
    
    
    def _add_next_piece_to_state(self, next_piece):
        self.board[:, :, -1] = np.zeros([4, 4], dtype=np.uint8)
        self.pieces[next_piece] = 0 
        coordi, coordj = self._int_to_coord(next_piece)
        self.board[coordi, coordj, -1] = 1


    def _get_reward(self):################################################################""
        if self.n_turns > MAX_LEN_EPISODE:
            self.done = True
            return -100
        if not self.valid_move:
            # print('Invalid move')
            self.valid_move = True
            return -1
        if self.score == 1:
            return 100
        elif self.score == -1:
            return -100
        else:
            return 0
        
    def _check_win(self, i, j):        
        colonne = self.board[:, j, :4].sum(axis=0)
        if 4 in colonne or (0 in colonne and 4 == self.board[:, j, 4].sum()):
            # print('colonne end')
            return True, 1
        
        ligne = self.board[i, :, :4].sum(axis=0)
        if 4 in ligne or (0 in ligne and 4 == self.board[i, :, 4].sum()):
            # print('ligne end')
            return True, 1
            
        diaggd = np.trace(self.board[:, :, :4], axis1=0, axis2=1)
        if 4 in diaggd or (0 in diaggd and 4 == np.trace(self.board[:, :, 4])):
            # print('diaggd end')
            return True, 1
            
        diagdg = np.trace(self.board[:, ::-1, :4], axis1=0, axis2=1)
        if 4 in diagdg or (0 in diagdg and 4 == np.trace(self.board[:, ::-1, 4])):
            # print('diaggdg end')
            return True, 1

        if self.board[:, :, 4].sum() == 16:
            # print('Game ended in a draw')
            return True, 0

        return False, None
    def action_masks(self):
        # place, piece
        mask = np.full(32, False)
        # Place masking
        mask[:16] = (self.board[:, :, 4] == 0).reshape(16)
        # Piece masking
        mask[16:] = self.pieces == 1
        return mask
        
    def _opponent_turn(self):
        if self._opponent == 'random':
            # place the piece at a random place
            lines, cols = np.where(self.board[:, :, 4] == 0)
            rd_move = random.randint(0, len(lines)-1)
            i, j = lines[rd_move], cols[rd_move]
            self._put_piece_on_board(i, j)
            self.done, self.score = self._check_win(i, j)
            if self.score:
                self.score = -self.score
            # Choosing a random piece
            pieces_left = np.nonzero(self.pieces)[0]
            if len(pieces_left) > 0:
                next_piece = np.random.choice(pieces_left)
                self._add_next_piece_to_state(next_piece)
        else:
            raise ValueError('Opponent type unknown')



    def _int_to_coord(self, value):
        return value//4, value%4
    
    def _coord_to_int(self, coord):
        return 4* coord[0] + coord[1]
    
    def _piece_to_binary(self, piece):
        return np.array(list(np.binary_repr(piece, width=4)), dtype=np.uint8)
    
    def _binary_to_piece(self, binary):
        # print('binary:', binary)
        if len(binary) == 5:
            if binary[-1] == 0: # No piece here
                return 0
            else:
                return int(''.join(map(str,binary[:-1])), base=2) + 1
        else:
            return int(''.join(map(str,binary)), base=2)
        
    
    def render(self, mode='human'):
        # Issue because piece number 0 is on the board
        # We swaped i, j when rendering the board
        rendered_board = [[self._binary_to_piece(self.board[i, j, :5]) for j in range(4)]for i in range(4)]
        print(np.array(rendered_board))
        print('Piece you have to play:', np.argmax(self.board[:, :, 5]))
        print(self.pieces, '\n')


def main():
    
    env = QuartoEnv(opponent='random')
    obs = env.reset()
    done = False
    
    while not done:
        # action = env.action_space.sample()
        env.render()
        action1 = int(input('choose where you place the piece:'))
        action2 = int(input('choose the piece you give:'))
        action = (action1, action2)
        obs, reward, done, info = env.step(action)
        print('Reward:', reward)
    env.render()
    
    




if __name__ == '__main__' :
    main()