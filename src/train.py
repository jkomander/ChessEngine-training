import argparse
import glob
import os
import pathlib
import shutil
import subprocess

def main():
    root_dir = 'C:\\Users\\jacki\\Documents\\Testing\\Train'
    engine_repos = 'C:\\Users\\jacki\\source\\repos\\jkomander\\ChessEngine'
    book = 'C:\\Users\\jacki\\Documents\\Testing\\openings\\books\\one_ply.pgn'
    training_repos = 'C:\\Users\\jacki\\source\\repos\\jkomander\\ChessEngine-training'

    concurrency = 12
    rounds = 3000000
    seed = 42

    time_per_game = 5
    increment_per_move = 0.05
    hash = 128
    threads = 1

    if os.path.exists(root_dir):
        shutil.rmtree(root_dir)

    assert os.path.isdir(engine_repos)
    assert os.path.isfile(book)
    assert os.path.isdir(training_repos)

    engine_repos_src = os.path.join(engine_repos, 'src')

    command = 'mingw32-make -C \"{}\" clean'.format(engine_repos_src)
    subprocess.run(command, shell=True)
    
    command = 'mingw32-make -C \"{}\"'.format(engine_repos_src)
    subprocess.run(command, shell=True)
    
    engine = glob.glob(
        os.path.join(engine_repos_src, '*exe')
    )[0]

    engine0_name = 'engine0'
    engine1_name = 'engine1'
    engine0 = os.path.join(root_dir, engine0_name + '.exe')
    engine1 = os.path.join(root_dir, engine1_name + '.exe')

    pathlib.Path(root_dir).mkdir()

    shutil.copy2(engine, engine0)
    shutil.copy2(engine, engine1)

    out_file = os.path.join(root_dir, 'out.pgn')

    command = ' '.join(
        [
        'cutechess-cli',
        '-engine', f'name={engine0_name}', f'cmd={engine0}',
        '-engine', f'name={engine1_name}', f'cmd={engine1}',
        '-pgnout', f'{out_file}',
        '-openings', f'file={book}', 'order=random',

        '-concurrency', f'{concurrency}',
        '-rounds', f'{rounds}',
        '-srand', f'{seed}',

        '-each', f'tc={time_per_game}+{increment_per_move}', 
        'proto=uci', f'option.Hash={hash}', f'option.Threads={threads}',
        ]
    )
    subprocess.run(command, shell=True)

    training_repos_src = os.path.join(training_repos, 'src')
    training_data = os.path.join(root_dir, 'out.td')

    command = 'python {} {} {}'.format(
        os.path.join(training_repos_src, 'pgn_to_td.py'),
        out_file,
        training_data
    )
    subprocess.run(command, shell=True)

if __name__ == '__main__':
    main()