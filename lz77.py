import sys

def lz77_compress(data, window_size=4096, lookahead_size=18):
    i = 0
    compressed = []

    while i < len(data):
        window_start = max(0, i - window_size)
        window = data[window_start:i]
        lookahead = data[i:i + lookahead_size]

        best_offset = 0
        best_length = 0

        for offset in range(1, len(window) + 1):
            max_possible_from_history = offset
            match_length = 0
            while (match_length < len(lookahead) and
                   match_length < 17 and
                   match_length < max_possible_from_history and
                   i - offset + match_length >= 0 and
                   data[i - offset + match_length] == lookahead[match_length]):
                match_length += 1

            if match_length >= 3 and match_length > best_length:
                best_length = match_length
                best_offset = offset

        if i + best_length < len(data):
            next_char = data[i + best_length:i + best_length + 1]  # bytes, not str
        else:
            next_char = b''

        if best_length >= 3 and best_length < i:
            token_len = best_length - 2
            compressed.append((best_offset, token_len, next_char))
            i += best_length + 1
        else:
            compressed.append((0, 0, data[i:i+1]))
            i += 1

    return compressed


def lz77_compress_to_bytes(data, window_size=4096, lookahead_size=18):
    tokens = lz77_compress(data, window_size, lookahead_size)
    binary_output = bytearray()

    for offset, token_len, next_char in tokens:
        if offset >= 4096:
            raise ValueError(f"Offset {offset} too large, max 4095")
        if token_len >= 16:
            raise ValueError(f"Token len {token_len} too large, max 15")

        char_byte = next_char[0] if next_char else 0

        byte0 = (offset >> 4) & 0xFF
        byte1 = ((offset & 0x0F) << 4) | (token_len & 0x0F)
        byte2 = char_byte & 0xFF

        binary_output.extend([byte0, byte1, byte2])

    return bytes(binary_output)

def lz77_decompress(compressed, original_size):
    result = bytearray()
    current_size = 0  # ← track how many bytes we've written

    for offset, token_len, next_char in compressed:
        # Calculate how many bytes this token will emit
        match_length = token_len + 2 if token_len > 0 else 0
        total_to_add = match_length + len(next_char)

        # 🚨 SAFETY: If adding would exceed original_size, cap it
        if current_size + total_to_add > original_size:
            # Cap match_length first
            if match_length > 0:
                allowed_match = max(0, original_size - current_size - len(next_char))
                match_length = min(match_length, allowed_match)
            # Cap next_char
            if len(next_char) > 0 and current_size + match_length >= original_size:
                next_char = b''  # skip it

        # Emit match
        if match_length > 0:
            start = len(result) - offset
            for j in range(match_length):
                if start + j >= 0:
                    result.append(result[start + j])
                else:
                    result.append(0)
            current_size += match_length

        # Emit next_char — only if within limit
        if len(next_char) > 0 and current_size < original_size:
            result.extend(next_char)
            current_size += len(next_char)

        # 🛑 Stop early if we hit limit
        if current_size >= original_size:
            break

    return bytes(result)

def lz77_decompress_from_bytes(binary_data, decompress_size):
    tokens = []
    i = 0
    n = len(binary_data)

    while i + 3 <= n:
        byte0 = binary_data[i]
        byte1 = binary_data[i+1]
        byte2 = binary_data[i+2]

        offset = (byte0 << 4) | (byte1 >> 4)
        token_len = byte1 & 0x0F
        # 🚨 ALWAYS include the byte — don't skip 0x00
        next_char = bytes([byte2])

        tokens.append((offset, token_len, next_char))
        i += 3

    return lz77_decompress(tokens, decompress_size)


# ✅ MODIFIED MAIN — READ FILE FROM COMMAND LINE
if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python lz77.py <input_file>")
        sys.exit(1)

    input_filename = sys.argv[1]
    compressed_filename = input_filename + ".lz77"
    decompressed_filename = input_filename + ".decomp"

    # Read input file as binary
    with open(input_filename, 'rb') as f:
        original_data = f.read()

    print(f"Original size: {len(original_data)} bytes")

    # Compress
    compressed_binary = lz77_compress_to_bytes(original_data)
    print(f"Compressed size: {len(compressed_binary)} bytes")
    print(f"Compression ratio: {len(compressed_binary)/len(original_data):.2%}")

    # ✅ Write header + compressed data — even if later decompression fails
        # ✅ Write 4-byte header: original_size ONLY
    try:
        with open(compressed_filename, 'wb') as f:
            f.write(len(original_data).to_bytes(8, byteorder='little'))  # only original size
            f.write(compressed_binary)
        print(f"Compressed data (+ 8-byte header) written to: {compressed_filename}")
    except Exception as e:
        print(f"⚠️  Error writing compressed file: {e}")

    try:
        # Decompress
        decompressed_data = lz77_decompress_from_bytes(compressed_binary, len(original_data))
        # Verif
        if original_data == decompressed_data:
            print("✅ Compression/decompression successful — data matches!")
        else:
            print("❌ ERROR: Data mismatch after decompression!")
            # Write decompressed data anyway for debugging
            with open(decompressed_filename + ".FAILED", 'wb') as f:
                f.write(decompressed_data)
                print(f"⚠️  Wrote failed decompression to: {decompressed_filename}.FAILED")
                sys.exit(1)
        # Write decompressed file
        with open(decompressed_filename, 'wb') as f:
            f.write(decompressed_data)
            print(f"Decompressed data written to: {decompressed_filename}")
    except Exception as e:
        print(f"⚠️  Error during decompression: {e}")
        sys.exit(1)
