import numpy as np


def verify_matrix_multiplication(A, B, C, rtol=1e-6, atol=1e-8):
    """
    Verify the result of matrix multiplication C = A * B

    Parameters:
    A, B -- original matrices
    C -- multiplication result to verify
    rtol, atol -- tolerances for comparison

    Returns:
    True -- if multiplication is correct
    False -- if there is an error
    """

    # 1. Type checking
    if not all(isinstance(m, (list, np.ndarray)) for m in [A, B, C]):
        print("❌ Error: Invalid input data type")
        return False

    # Convert to numpy arrays
    A = np.array(A, dtype=float)
    B = np.array(B, dtype=float)
    C = np.array(C, dtype=float)

    # 2. Dimension checking
    if A.shape[1] != B.shape[0]:
        print(
            f"❌ Error: Cannot multiply A({A.shape[0]}x{A.shape[1]}) and B({B.shape[0]}x{B.shape[1]})")
        return False

    expected_shape = (A.shape[0], B.shape[1])
    if C.shape != expected_shape:
        print(f"❌ Dimension error: expected {expected_shape}, got {C.shape}")
        return False

    # 3. Calculate the correct result
    expected_C = np.dot(A, B)  # or A @ B

    # 4. Compare results
    if np.allclose(C, expected_C, rtol=rtol, atol=atol):
        print("✅ Verification passed: matrix multiplication is correct")
        return True
    else:
        print("❌ Verification failed: matrix multiplication is uncorrect")
        return False


def read_matrix_from_file(filename):
    """
    Read a matrix from a file where the first line contains the matrix size.
    File format:
    n m
    a11 a12 ... a1m
    a21 a22 ... a2m
    ...
    an1 an2 ... anm

    Parameters:
    filename -- path to the file

    Returns:
    matrix as numpy array
    """

    try:
        with open(filename, 'r') as file:
            # Read the first line with dimensions
            first_line = file.readline().strip()
            if not first_line:
                print(f"❌ Error: File {filename} is empty")
                return None

            # Parse dimensions
            dimensions = first_line.split()
            if len(dimensions) != 2:
                print("❌ Error: First line must contain 2 numbers (rows and columns)")
                print(f"   Got: {first_line}")
                return None

            try:
                rows = int(dimensions[0])
                cols = int(dimensions[1])
            except ValueError:
                print(f"❌ Error: Invalid dimensions format: {first_line}")
                return None

            if rows <= 0 or cols <= 0:
                print(f"❌ Error: Invalid dimensions: {rows}x{cols}")
                return None

            # Read the matrix data
            matrix = []
            for i in range(rows):
                line = file.readline()
                if not line:
                    print(f"❌ Error: File has fewer than {rows} rows of data")
                    return None

                # Parse row values
                row_values = line.strip().split()
                if len(row_values) != cols:
                    print(
                        f"❌ Error: Row {i+1} has {len(row_values)} values, expected {cols}")
                    return None

                try:
                    row = [float(x) for x in row_values]
                    matrix.append(row)
                except ValueError:
                    print(
                        f"❌ Error: Invalid number format in row {i+1}: {line}")
                    return None

            print(f"✅ Successfully read {rows}x{cols} matrix from {filename}")
            return np.array(matrix)

    except FileNotFoundError:
        print(f"❌ Error: File {filename} not found")
        return None
    except Exception as e:
        print(f"❌ Error reading file: {e}")
        return None


if __name__ == "__main__":
    A = read_matrix_from_file('Matrix_A.txt')
    B = read_matrix_from_file('Matrix_B.txt')
    C = read_matrix_from_file('Matrix_C.txt')
    verify_matrix_multiplication(A, B, C)
