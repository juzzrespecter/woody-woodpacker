import gdb

class ParseDynamic(gdb.Command):
    """Parse .dynamic section of ELF in GDB"""
    
    def __init__(self):
        super(ParseDynamic, self).__init__("parse-dynamic", gdb.COMMAND_DATA)
        
    def invoke(self, args, from_tty):
        # Parse arguments: parse-dynamic <address> [size]
        args = args.split()
        if len(args) < 1:
            print("Usage: parse-dynamic <dynamic_section_address> [size]")
            return
            
        dyn_addr = int(args[0], 0)  # Support 0x format
        size = int(args[1], 0) if len(args) > 1 else 0x1000  # Default size
        
        # Dynamic tag constants
        DT_NAMES = {
            0: "DT_NULL",      1: "DT_NEEDED",    2: "DT_PLTRELSZ",  3: "DT_PLTGOT",
            4: "DT_HASH",      5: "DT_STRTAB",    6: "DT_SYMTAB",    7: "DT_RELA",
            8: "DT_RELASZ",    9: "DT_RELAENT",   10: "DT_STRSZ",    11: "DT_SYMENT",
            12: "DT_INIT",     13: "DT_FINI",     14: "DT_SONAME",   15: "DT_RPATH",
            16: "DT_SYMBOLIC", 17: "DT_REL",      18: "DT_RELSZ",    19: "DT_RELENT",
            20: "DT_PLTREL",   21: "DT_DEBUG",    22: "DT_TEXTREL",  23: "DT_JMPREL",
            24: "DT_BIND_NOW", 25: "DT_INIT_ARRAY", 26: "DT_FINI_ARRAY", 27: "DT_INIT_ARRAYSZ",
            28: "DT_FINI_ARRAYSZ", 29: "DT_RUNPATH", 30: "DT_FLAGS", 31: "DT_ENCODING",
            33: "DT_PREINIT_ARRAY", 34: "DT_PREINIT_ARRAYSZ", 35: "DT_SYMTAB_SHNDX"
        }
        
        print(f"Parsing .dynamic section at {dyn_addr:#x}")
        print(f"{'Offset':<10} {'Type':<15} {'Value':<18} {'Name':<20}")
        print("-" * 70)
        
        offset = 0
        while True:
            try:
                # Read d_tag (8 bytes)
                d_tag = int(gdb.parse_and_eval(f"*(long*)({dyn_addr + offset})"))
                # Read d_val/d_ptr (8 bytes)  
                d_val = int(gdb.parse_and_eval(f"*(long*)({dyn_addr + offset + 8})"))
                
                tag_name = DT_NAMES.get(d_tag, f"DT_UNKNOWN({d_tag:#x})")
                
                print(f"{dyn_addr + offset:#8x} {d_tag:#10x} {d_val:#18x} {tag_name:<20}")
                
                if d_tag == 0:  # DT_NULL - end of dynamic section
                    break
                    
                offset += 16  # Each entry is 16 bytes (d_tag + d_val)
                
                if offset >= size:  # Safety check
                    print("Warning: Reached size limit without finding DT_NULL")
                    break
                    
            except:
                print(f"Error reading memory at {dyn_addr + offset:#x}")
                break

# Register the command
ParseDynamic()
