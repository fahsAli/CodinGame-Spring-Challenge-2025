use std::collections::HashMap;
use std::io::{self, BufRead};

const MOD: u32 = 1 << 30;

#[inline(always)]
fn get_cell(state: u32, pos: usize) -> u32 {
    (state >> (pos * 3)) & 7
}

#[inline(always)]
fn set_cell(state: u32, pos: usize, value: u32) -> u32 {
    (state & !(7 << (pos * 3))) | ((value & 7) << (pos * 3))
}

#[inline(always)]
fn hash_board(state: u32) -> u32 {
    let mut h = get_cell(state, 0);
    for pos in 1..9 {
        h = h * 10 + get_cell(state, pos);
    }
    h & (MOD - 1)
}

#[inline(always)]
fn is_full(state: u32) -> bool {
    (0..9).all(|pos| get_cell(state, pos) != 0)
}

#[inline(always)]
fn add_to_map(dp: &mut HashMap<u32, u32>, state: u32, ways: u32) {
    dp.entry(state)
        .and_modify(|count| *count = (*count + ways) % MOD)
        .or_insert(ways % MOD);
}

const NEIGHBORS: [[i8; 4]; 9] = [
    [1, 3, -1, -1], [0, 2, 4, -1], [1, 5, -1, -1],
    [0, 4, 6, -1], [1, 3, 5, 7], [2, 4, 8, -1],
    [3, 7, -1, -1], [4, 6, 8, -1], [5, 7, -1, -1],
];

const NEIGHBOR_COUNT: [usize; 9] = [2, 3, 2, 3, 4, 3, 2, 3, 2];

#[derive(Debug, Clone)]
struct CaptureSet {
    combos: Vec<Vec<usize>>,
}

static mut CAPTURE_SETS: Option<[CaptureSet; 9]> = None;

fn precompute_captures() -> [CaptureSet; 9] {
    let mut table: [CaptureSet; 9] = std::array::from_fn(|_| CaptureSet { combos: Vec::new() });

    for pos in 0..9 {
        let nc = NEIGHBOR_COUNT[pos];
        let neighbors = NEIGHBORS[pos];

        for i in 0..nc {
            for j in (i + 1)..nc {
                table[pos].combos.push(vec![
                    neighbors[i] as usize,
                    neighbors[j] as usize,
                ]);
            }
        }

        if nc >= 3 {
            for i in 0..nc {
                for j in (i + 1)..nc {
                    for k in (j + 1)..nc {
                        table[pos].combos.push(vec![
                            neighbors[i] as usize,
                            neighbors[j] as usize,
                            neighbors[k] as usize,
                        ]);
                    }
                }
            }
        }

        if nc == 4 {
            table[pos].combos.push(neighbors[..4].iter().map(|&n| n as usize).collect());
        }
    }

    table
}

fn process_transitions(state: u32, ways: u32, pos: usize, next_dp: &mut HashMap<u32, u32>, capture_sets: &[CaptureSet; 9]) {
    let mut captured = false;
    let combos = &capture_sets[pos].combos;

    for combo in combos {
        let mut sum = 0;
        let mut valid = true;

        for &neighbor in combo {
            let val = get_cell(state, neighbor);
            if val == 0 {
                valid = false;
                break;
            }
            sum += val;
        }

        if valid && sum <= 6 {
            captured = true;
            let mut new_state = state;
            for &neighbor in combo {
                new_state = set_cell(new_state, neighbor, 0);
            }
            new_state = set_cell(new_state, pos, sum);
            add_to_map(next_dp, new_state, ways);
        }
    }

    if !captured {
        let new_state = set_cell(state, pos, 1);
        add_to_map(next_dp, new_state, ways);
    }
}

fn main() {
    unsafe {
        CAPTURE_SETS = Some(precompute_captures());
    }

    let stdin = io::stdin();
    let mut lines = stdin.lock().lines();

    let max_depth: usize = lines.next().unwrap().unwrap().trim().parse().unwrap();

    let mut init_state: u32 = 0;
    let mut values = Vec::new();
    while values.len() < 9 {
        if let Some(Ok(line)) = lines.next() {
            for token in line.split_whitespace() {
                if let Ok(val) = token.parse() {
                    values.push(val);
                }
            }
        }
    }

    for i in 0..9 {
        init_state = set_cell(init_state, i, values[i]);
    }

    let mut current_dp = HashMap::new();
    current_dp.insert(init_state, 1u32);

    let mut result = 0u32;
    for d in 0..=max_depth {
        let mut next_dp = HashMap::new();

        for (&state, &ways) in current_dp.iter() {
            if d == max_depth || is_full(state) {
                result = (result + ways * hash_board(state)) % MOD;
                continue;
            }

            for pos in 0..9 {
                if get_cell(state, pos) == 0 {
                    unsafe {
                        if let Some(ref sets) = CAPTURE_SETS {
                            process_transitions(state, ways, pos, &mut next_dp, sets);
                        }
                    }
                }
            }
        }

        if d == max_depth {
            break;
        }

        current_dp = next_dp;
    }

    println!("{}", result);
}
