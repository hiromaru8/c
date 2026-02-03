import ctypes
from ctypes import c_char_p, c_int, c_ubyte, POINTER

DLL_PATH = "./build/libsqlite_export.dll"

dll = ctypes.CDLL(DLL_PATH)

# se_query の定義
dll.se_query.argtypes = [
    c_char_p,                      # db_path
    c_char_p,                      # table
    c_char_p,                      # pk_list
    c_char_p,                      # pk_values
    c_char_p,                      # col_list
    POINTER(POINTER(c_ubyte)),     # out_buf
    POINTER(c_int)                 # out_size
]
dll.se_query.restype = c_int

# se_update の定義
dll.se_update.argtypes = [
    c_char_p,   # db_path
    c_char_p,   # table
    c_char_p,   # pk_list
    c_char_p,   # pk_values
    c_char_p,   # set_clause
    POINTER(c_int)
]
dll.se_update.restype = c_int




def test_se_query(db_path=b"sample_test.db",
                  table=b"tb1",
                  pk_list=b"id_src,id_dest",
                  pk_values=b"(1,10),(2,20)",
                  col_list=b"data1,data2"):

    out_buf = POINTER(c_ubyte)()
    out_size = c_int()

    result = dll.se_query(
        db_path,
        table,
        pk_list,
        pk_values,
        col_list,
        ctypes.byref(out_buf),
        ctypes.byref(out_size)
    )

    assert result == 0, f"se_query failed with error code {result}"
    assert out_size.value > 0, "Output size should be greater than 0"

    # Convert output buffer to bytes
    output_bytes = ctypes.string_at(out_buf, out_size.value)

    # Free the allocated buffer if necessary (depends on DLL implementation)
    # dll.free_buffer(out_buf)

    print("Output Size:", out_size.value)
    print("Output Bytes:", output_bytes.hex())
    
    
def test_update_se_query(db_path=b"sample_test.db",
                         table=b"tb1",
                         pk_list=b"id_src,id_dest",
                         pk_values=b"(1,10),(2,20)",
                         set_clause=b"data1=X'0000', data2=X'ffff'"):
    affected = c_int()
    result = dll.se_update(
        db_path,
        table,
        pk_list,
        pk_values,
        set_clause,
        ctypes.byref(affected)
    )
    assert result == 0, f"se_update failed with error code {result}"
    assert affected.value == 2, f"Expected 2 rows to be affected, got {affected.value}"
    print("Rows affected:", affected.value)
    

if __name__ == "__main__":
    test_se_query()
    test_se_query(table=b"tb1", pk_list=b"id_src", pk_values=b"(1)", col_list=b"data1,data2")
    test_se_query(table=b"tb2", pk_list=b"id", pk_values=b"(1),(2)", col_list=b"data1,data2")
    
    print("\n--- Performing Update ---\n")
    test_update_se_query(table=b"tb1", pk_list=b"id_src", pk_values=b"(1)", set_clause=b"data1=X'0000', data2=X'ffff'")
    test_se_query(table=b"tb1", pk_list=b"id_src", pk_values=b"(1)", col_list=b"data1,data2")