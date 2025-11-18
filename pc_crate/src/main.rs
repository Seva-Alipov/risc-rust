use std::env;
use std::process::Stdio;
use std::process::Command;

fn main() {
    let mut args: Vec<String> = env::args().collect();
    args[0] = "../build/compiler".to_string();

    if args.len() == 2 {
        args.push(args[1].replace(".rs", ".hex"));
    }
    else if args.len() == 1 {
        println!("No args provided, using default value of example.rs");
        args.push("./to_board/example.rs".to_string());
        args.push("./to_board/example.hex".to_string());
    }
    else if args.len() > 3 {
        panic!("Too many arguments.");
    }

    let status = Command::new(&args[0])
        .args(&args[1..]) // skip the first element when passing args
        .stdin(Stdio::inherit())
        .stdout(Stdio::inherit())
        .stderr(Stdio::inherit())
        .status()
        .expect("failed to execute command");

    if !status.success() {
        let errcode = status.code().unwrap();
        eprintln!("Compiler failed with code {errcode}.");
        panic!("");
    }
}