// Usage: ./rv32_min <input.rs> <output.hex>
//
// Requirements:
//   - rustc in PATH
//      - to install go to rustup.rs for instructions
//      - with target riscv32imac-unknown-none-elf
//          - to add target     > rustup target add riscv32imac-unknown-none-elf
//   - binutils-riscv64-unknown-elf and gcc-riscv64-unknown-elf in PATH
//      - to install    > sudo apt install binutils-riscv64-unknown-elf gcc-riscv64-unknown-elf
// Notes:
//   - This assumes a single-file bare-metal Rust program (#![no_std], defines an entry symbol, e.g. _start).
//   - For custom memory layout, pass -Clink-arg=-Tmemory.x via RUSTFLAGS env if needed.

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>

#define WORD_LENGTH 4
#define TOTAL_BYTES 1024
#define TOTAL_WORDS TOTAL_BYTES/WORD_LENGTH

static void die(const char *msg) { perror(msg); exit(errno); }

static void run(char *const argv[]) {
    pid_t pid = fork();
    if (pid < 0) die("fork");
    if (pid == 0) { execvp(argv[0], argv); _exit(127); }
    int st = 0;
    if (waitpid(pid, &st, 0) < 0) die("waitpid");
    if (!WIFEXITED(st) || WEXITSTATUS(st) != 0) {
        fprintf(stderr, "command failed: %s\n", argv[0]);
        exit(1);
    }
}

int bin_to_wordhex(const char *bin_path, const char *hex_path) {
    FILE *in = fopen(bin_path, "rb");
    if (!in) {
        fprintf(stderr, "Error: could not open input file.\n");
        return -1;
    }

    FILE *out = fopen(hex_path, "w");
    if (!out) {
        fclose(in);
        fprintf(stderr, "Error: could not open output file.\n");
        return -2;
    }

    uint8_t byte[WORD_LENGTH];
    size_t words_written = 0;

    while (words_written < TOTAL_WORDS) {
        size_t len = fread(byte, 1, WORD_LENGTH, in);

        if (len == 0) {
            // End of file reached → stop reading and emit zeros
            memset(byte, 0, WORD_LENGTH);
        }
        else if (len < WORD_LENGTH) {
            // Partially read word → pad with zeros
            for (size_t i = len; i < WORD_LENGTH; i++)
                byte[i] = 0;
        }

        // Convert to little-endian 32-bit word
        uint32_t word =  (uint32_t)byte[0]
                      | ((uint32_t)byte[1] << 8)
                      | ((uint32_t)byte[2] << 16)
                      | ((uint32_t)byte[3] << 24);

        fprintf(out, "%08X\n", word);
        words_written++;

        // If we read a *full* word and the file still has more data
        // after reaching the maximum size → file is too big
        if (len == WORD_LENGTH && words_written == TOTAL_WORDS) {
            // If there is more data in the file, print error
            uint8_t extra;
            if (fread(&extra, 1, 1, in) == 1) {
                printf("The binary file is too long. You may still run it, but doing so may result in undefined behaviour.\n");
            }
        }
    }

    fclose(in);
    fclose(out);
    return 0;
}

int main(int argc, char **argv) {
    /*if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.rs> <output.bin>\n", argv[0]);
        return 1;
    }*/
    const char *out_hex = "../src_verilog/firmware.hex";

    // Now with crate, elf is in same place always
    const char *out_elf = "../rv32_crate/target/riscv32imac-unknown-none-elf/release/rv32_crate";

    char *cargo_argv[] = {
        "cargo",
        "build",
        "--release",
        NULL
    };

    run(cargo_argv);

    // 2) ELF -> Verilog plaintext HEX (GNU objcopy required)
    const char *objcopy = NULL;
    if (access("/usr/bin/riscv64-unknown-elf-objcopy", X_OK) == 0) {
        objcopy = "/usr/bin/riscv64-unknown-elf-objcopy";
    } else {
        die("gnu objectcopy required");
    }

    char out_bin[4096];
    snprintf(out_bin, sizeof(out_bin), "%s.bin", out_hex);

    char *argv_gnu[] = {
        (char*)objcopy,
        "-O", "binary",
        "-S",
        "-j", ".text", "-j", ".rodata", "-j", ".data",
        (char*)out_elf, (char*)out_bin, NULL
    };

    run(argv_gnu);

    // then convert out_bin -> out_hex (word-per-line, LE)
    if(bin_to_wordhex(out_bin, out_hex) != 0) {
        die("Could not create word per line hex.\n");
    }

    fprintf(stderr, "OK: %s\n", out_hex);
    int removed = remove(out_bin);
    if(removed == 0) fprintf(stderr, "Cleanup successful.\n");
    else fprintf(stderr, "Cleanup failed.\n");
    return 0;
}

