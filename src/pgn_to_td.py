import argparse
import chess
import chess.pgn
import math
import os
import tqdm

from constants import*

class PGN_Converter:
    def __init__(self, source, target):
        self.source = source
        self.target = target
        self.pgn = open(source)
        self.buffer = bytearray()
        self.numInvalidScores = 0

    def convert(self):
        num_games = 0
        while True:
            game = chess.pgn.read_game(self.pgn)
            if game:
                num_games += 1
            else:
                break
        self.pgn.close()
        
        self.pgn = open(self.source)
        for i in tqdm.tqdm(range(num_games)):
            game = chess.pgn.read_game(self.pgn)
            if game:
                self.process_game(game)
            else:
                break
            
        if (self.numInvalidScores):
            print('WARNING: Number of invalid scores: {}.'.format(self.numInvalidScores))

    def process_game(self, game):
        game_result = __class__.game_result(game.headers['Result'])
        board = game.board()

        for node in game.mainline():
            score = __class__.score(node.comment)

            if score == None or abs(score) > MAX_EVAL_SCORE and abs(score) < MIN_MATE_SCORE or abs(score) > MATE_SCORE:
                self.numInvalidScores += 1
                return
        
            fen = node.parent.board().fen()
            self.write_entry(fen, score, game_result if node.parent.turn() == chess.WHITE else - game_result)

    def write_entry(self, fen, score, game_result):
        self.buffer.extend(len(fen).to_bytes(length=1, byteorder='little'))
        self.buffer.extend(bytes(fen, 'utf-8'))
        self.buffer.extend(score.to_bytes(length=2, byteorder='little', signed=True))
        self.buffer.extend(game_result.to_bytes(length=1, byteorder='little', signed=True))

    def write(self):
        with open(self.target, 'wb') as f:
            f.write(self.buffer)

    @staticmethod
    def game_result(str):
        match str:
            case '0-1':
                return -1
            case '1/2-1/2':
                return 0
            case '1-0':
                return 1
        assert False

    @staticmethod
    def score(str):
        if (str[0] == '\n'):
            str = str[1:]
            
        if not str.find('/'):
            return None

        score_str = str.split('/')[0]

        if not (score_str[0] == '+' or score_str[0] == '-' or float(score_str) == 0):
            return None

        negative = score_str[0] == '-'

        if score_str[1] == 'M':
            score = MATE_SCORE - int(score_str[2:])
        
        else:
            score = int(100 * float(score_str[1:]))
            
        if negative:
            return -score
        else:
            return score

def main():
    parser = argparse.ArgumentParser(description='Converts files from pgn to td format')
    parser.add_argument('source', help='Source file (.pgn)')
    parser.add_argument('target', help='Target file (.td)')
    args = parser.parse_args()

    print('Converting {} to {}.'.format(args.source, args.target))

    if (args.source.endswith('.pgn') and args.target.endswith('.td')):
        converter = PGN_Converter(
            os.path.abspath(args.source),
            os.path.abspath(args.target)
        )
        converter.convert()
        converter.write()

    else:
        raise Exception('Invalid file types: ', str(args))

if __name__ == '__main__':
    main()