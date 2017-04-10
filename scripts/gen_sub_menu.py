#!/usr/bin/env python

import os
from os.path import abspath, join, dirname

root = abspath(join(dirname(__file__), ".."))
app_path = join(root, "app")
board_path = join(root, "board")


def get_path_dir(path):
    files = os.listdir(path)
    dirs = []

    for file in files:
        if os.path.isdir(join(path, file)):
            dirs.append(file)
    return dirs


def gen_board_kconfig(boards):
    with open(join(board_path, "Kconfig"), 'w') as kfd:
        kfd.write("#\n# Automatically generated by 'make config'. don't edit\n#\n\n")

        for vendor in boards:
            kfd.write("if BOARD_" + vendor.upper() + "\n\n")
            kfd.write("choice\n\tprompt \"Select boards\"\n\n")

            boards_name = boards[vendor]
            for board in boards_name:
                kfd.write("config TARGET_" + board.upper() +"\n")
                kfd.write("\tbool \"" + board.upper() + "\"\n\n")

            kfd.write("endchoice\n\n")

            for board in boards_name:
                file_name = join("board", vendor, board, "Kconfig")
                if os.path.isfile(join(root, file_name)):
                    kfd.write("source " + file_name + "\n")

            kfd.write("\nendif\n\n\n")


def gen_app_kconfig(apps):
    with open(join(app_path, "Kconfig"), 'w') as kfd:
        kfd.write("#\n# Automatically generated by 'make config'. don't edit.\n#\n\n")
        kfd.write("menu \"Select application\"\n\n")
        kfd.write("choice\n\tprompt \"Application\"\n\n")

        for kind in apps:
            kfd.write("config APP_" + kind.upper() + "\n")
            kfd.write("\tbool \"" + kind + "\"\n\n")

        kfd.write("endchoice\n\n")

        for kind in apps:
            kfd.write("choice\n")
            kfd.write("\tprompt \"" + kind + "\"\n")
            kfd.write("\tdepends on APP_" + kind.upper() + "\n\n")
            app_names = apps[kind]
            for name in app_names:
                kfd.write("config " + kind.upper() + "_" + name.upper() + "\n")
                kfd.write("\tbool \"" + kind + "_" + name + "\"\n\n")
            kfd.write("endchoice\n\n")

        kfd.write("config APP_DIR\n\tstring\n")
        for kind in apps:
            app_names = apps[kind]
            for name in app_names:
                kfd.write("\tdefault \"" + kind + "/" + name + "\" ")
                kfd.write("if " + kind.upper() + "_" + name.upper() + "\n")

        kfd.write("\nendmenu\n\n")


# generate board Kconfig
board_vendor = get_path_dir(board_path)
board = {}
for vendor in board_vendor:
    board[vendor] = get_path_dir(join(board_path, vendor))
gen_board_kconfig(board)

# generate app Kconfig
app_kind = get_path_dir(app_path)
app = {}
for kind in app_kind:
    app[kind] = get_path_dir(join(app_path, kind))
gen_app_kconfig(app)
