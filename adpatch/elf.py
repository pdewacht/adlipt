from collections import namedtuple
import struct


ElfHeader = namedtuple(
    'ElfHeader',
    'e_ident e_type e_machine e_version e_entry e_phoff e_shoff e_flags '
    'e_ehsize e_phentsize e_phnum e_shentsize e_shnum e_shstrndx')


SectionHeader = namedtuple(
    'SectionHeader',
    'sh_name sh_type sh_flags sh_addr sh_offset sh_size sh_link sh_info '
    'sh_addralign sh_entsize')


SymbolTableEntry = namedtuple(
    'SymbolTableEntry',
    'st_name st_value st_size st_info st_other st_shndx')


RelocationEntry = namedtuple(
    'RelocationEntry',
    'r_offset r_type r_sym')


def parse_elf_header(bs, offset=0):
    return ElfHeader(*struct.unpack_from('<16s 2H 5I 6H', bs, offset))


def parse_section_header(bs, offset):
    return SectionHeader(*struct.unpack_from('<10I', bs, offset))


def parse_symbol_table_entry(bs, offset):
    return SymbolTableEntry(*struct.unpack_from('<3I 2B H', bs, offset))


def parse_relocation_entry(bs, offset):
    return RelocationEntry(*struct.unpack(
        '<IBI', bs[offset:offset+8] + b'\x00'))


def asciiz(bs, offset):
    return bs[offset:bs.index(b'\x00', offset)].decode('ascii')


RELOCATION_TYPES = {
    1: 'R_386_32',
    2: 'R_386_PC32',
    20: 'R_386_16',
    21: 'R_386_PC16',
}


def parse(bs):
    elf = parse_elf_header(bs)
    section_list = [
        parse_section_header(bs, elf.e_shoff + i * elf.e_shentsize)
        for i in range(elf.e_shnum)
    ]

    def section_name(n):
        return asciiz(bs, section_list[elf.e_shstrndx].sh_offset + n)

    sections = {section_name(s.sh_name): s for s in section_list}

    s = sections['.symtab']
    symbols = [
        parse_symbol_table_entry(bs, s.sh_offset + i)
        for i in range(0, s.sh_size, s.sh_entsize)
    ]

    def symbol_name(sym):
        return asciiz(bs, sections['.strtab'].sh_offset + sym.st_name)

    exports = {
        symbol_name(sym): sym.st_value
        for sym in symbols
        if sym.st_info & 16 != 0
        if section_name(sym.st_shndx) == '.text'
    }

    if '.rel.text' not in sections:
        relocations = []
    else:
        s = sections['.rel.text']
        relocations = [
            parse_relocation_entry(bs, s.sh_offset + i)
            for i in range(0, s.sh_size, s.sh_entsize)
        ]

        relocations = [
            RelocationEntry(
                r_offset=rel.r_offset,
                r_type=RELOCATION_TYPES[rel.r_type],
                r_sym=symbol_name(symbols[rel.r_sym])
            )
            for rel in relocations
        ]

    s = sections['.text']
    text = bs[s.sh_offset:s.sh_offset+s.sh_size]

    return (text, exports, relocations)
