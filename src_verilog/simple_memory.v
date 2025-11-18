module simple_memory (
    input  wire        clk,
    input  wire        resetn,

    // PicoRV32 memory interface
    input  wire        mem_valid,
    input  wire        mem_instr,
    output reg         mem_ready,
    input  wire [31:0] mem_addr,
    input  wire [31:0] mem_wdata,
    input  wire [3:0]  mem_wstrb,
    output reg  [31:0] mem_rdata,
    output reg  [9:0] LEDR
);

    // Assigning only the exact amount of memory needed
    reg [31:0] memory [255:0];

    // Initialize memory from firmware.hex
    initial begin
        $readmemh("firmware.hex", memory);
    end

    // Address decoding (word-aligned)

    wire [9:0] word_addr = mem_addr[11:2];  // 2^10 = 1024 words

    always @(posedge clk) begin
        mem_ready <= 0;

        if (mem_valid && !mem_ready) begin
            mem_ready <= 1;

            if(mem_addr[31:20] == 12'hFF2)begin
                LEDR <= mem_wdata[9:0];
            end
            else begin
            // Read
            mem_rdata <= memory[word_addr];

            // Write with byte strobes
            if (mem_wstrb[0]) memory[word_addr][7:0]   <= mem_wdata[7:0];
            if (mem_wstrb[1]) memory[word_addr][15:8]  <= mem_wdata[15:8];
            if (mem_wstrb[2]) memory[word_addr][23:16] <= mem_wdata[23:16];
            if (mem_wstrb[3]) memory[word_addr][31:24] <= mem_wdata[31:24];
            end

        end
    end

endmodule
