import argparse
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
        '--engine',
        type=str,
        help='Path to chess engine executable'
    )
    parser.add_argument(
        '--book',
        type=str,
        help='Path to opening book'
    )
    parser.add_argument(
        '--time',
        type=float,
        default=10
    )
    parser.add_argument(
        '--increment',
        type=float,
        default=0.1
    )
    parser.add_argument(
        '--hash',
        type=int,
        default=128,
    )
    parser.add_argument(
        '--threads',
        type=int,
        default=1
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
    args = parser.parse_args()

    assert os.path.isfile(args.engine)
    assert os.path.isfile(args.book)

    out_file = os.path.join(args.root_dir, 'out.pgn')

    engine_basename = os.path.basename(args.engine)
    engine0_name = 'engine0'
    engine1_name = 'engine1'
    engine0_file = os.path.join(args.root_dir, engine0_name + '.exe')
    engine1_file = os.path.join(args.root_dir, engine1_name + '.exe')

    pathlib.Path(args.root_dir).mkdir()

    shutil.copy2(args.engine, args.root_dir)
    os.rename(
        os.path.join(args.root_dir, engine_basename),
        engine0_file
    )

    shutil.copy2(args.engine, args.root_dir)
    os.rename(
        os.path.join(args.root_dir, engine_basename),
        engine1_file
    )

    command = ['cutechess-cli',
               '-engine', f'name={engine0_name}', f'cmd={engine0_file}',
               '-engine', f'name={engine1_name}', f'cmd={engine1_file}',
               '-pgnout', f'{out_file}',
               '-openings', f'file={args.book}', 'order=random',
               '-each', f'tc={args.time}+{args.increment}', 'proto=uci', f'option.Hash={args.hash}', f'option.Threads={args.threads}',
               '-concurrency', f'{args.concurrency}',
               '-rounds', f'{args.rounds}',
               '-srand', f'{args.seed}',
               ]

    process = subprocess.Popen(' '.join(command), shell=True)
    process.wait()

if __name__ == '__main__':
    main()