import argparse
import glob
import os
import pathlib
import shutil
import subprocess

def main():
    parser = argparse.ArgumentParser(
        formatter_class = argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        '--root_dir',
        type=str
    )
    parser.add_argument(
        '--engine_repos',
        type=str,
        help='Path to chess engine repository'
    )
    parser.add_argument(
        '--book',
        type=str,
        help='Path to opening book'
    )
    parser.add_argument(
        '--training_repos',
        type=str,
        help='Path to training repository'
    )
    parser.add_argument(
        '--concurrency',
        type=int,
        default=12
    )
    parser.add_argument(
        '--rounds',
        type=int,
        default=50000000
    )
    parser.add_argument(
        '--seed',
        type=int,
        default=42
    )
    parser.add_argument(
        '--time_per_game',
        type=float,
        default=5
    )
    parser.add_argument(
        '--increment_per_move',
        type=float,
        default=0.05
    )
    parser.add_argument(
        '--hash',
        type=int,
        default=64,
    )
    parser.add_argument(
        '--threads',
        type=int,
        default=1
    )
    args = parser.parse_args()

    if os.path.exists(args.root_dir):
        shutil.rmtree(args.root_dir)

    assert os.path.isdir(args.engine_repos)
    assert os.path.isfile(args.book)
    assert os.path.isdir(args.training_repos)

    engine_repos_src = os.path.join(args.engine_repos, 'src')

    command = 'mingw32-make -C \"{}\" clean'.format(engine_repos_src)
    subprocess.run(command, shell=True)
    
    command = 'mingw32-make -C \"{}\"'.format(engine_repos_src)
    subprocess.run(command, shell=True)
    
    engine = glob.glob(
        os.path.join(engine_repos_src, '*exe')
    )[0]

    engine0_name = 'engine0'
    engine1_name = 'engine1'
    engine0 = os.path.join(args.root_dir, engine0_name + '.exe')
    engine1 = os.path.join(args.root_dir, engine1_name + '.exe')

    pathlib.Path(args.root_dir).mkdir()

    shutil.copy2(engine, engine0)
    shutil.copy2(engine, engine1)

    out_file = os.path.join(args.root_dir, 'out.pgn')

    command = ' '.join(
        [
        'cutechess-cli',
        '-engine', f'name={engine0_name}', f'cmd={engine0}',
        '-engine', f'name={engine1_name}', f'cmd={engine1}',
        '-pgnout', f'{out_file}',
        '-openings', f'file={args.book}', 'order=random',

        '-concurrency', f'{args.concurrency}',
        '-rounds', f'{args.rounds}',
        '-srand', f'{args.seed}',

        '-each', f'tc={args.time_per_game}+{args.increment_per_move}', 
        'proto=uci', f'option.Hash={args.hash}', f'option.Threads={args.threads}',
        ]
    )
    subprocess.run(command, shell=True)

if __name__ == '__main__':
    main()