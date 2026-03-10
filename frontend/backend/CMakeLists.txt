cmake_minimum_required(VERSION 3.16)
project(extreamfs)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/core
    ${PROJECT_SOURCE_DIR}/core/mount
    ${PROJECT_SOURCE_DIR}/core/commands
    ${PROJECT_SOURCE_DIR}/core/filesystem
    ${PROJECT_SOURCE_DIR}/core/disk
    ${PROJECT_SOURCE_DIR}/core/reports
)

set(SOURCES
    ${PROJECT_SOURCE_DIR}/include/main.cpp
    ${PROJECT_SOURCE_DIR}/core/commands/mkdisk.cpp
    ${PROJECT_SOURCE_DIR}/core/commands/fdisk.cpp
    ${PROJECT_SOURCE_DIR}/core/commands/rmdisk.cpp
    ${PROJECT_SOURCE_DIR}/core/commands/mount.cpp
    ${PROJECT_SOURCE_DIR}/core/commands/mkfs.cpp
    ${PROJECT_SOURCE_DIR}/core/commands/mkgrp.cpp
    ${PROJECT_SOURCE_DIR}/core/commands/rmgrp.cpp
    ${PROJECT_SOURCE_DIR}/core/commands/mkusr.cpp
    ${PROJECT_SOURCE_DIR}/core/commands/rmusr.cpp
    ${PROJECT_SOURCE_DIR}/core/commands/chgrp.cpp
    ${PROJECT_SOURCE_DIR}/core/commands/mkdir_cmd.cpp
    ${PROJECT_SOURCE_DIR}/core/commands/mkfile_cmd.cpp
    ${PROJECT_SOURCE_DIR}/core/commands/cat_cmd.cpp
    ${PROJECT_SOURCE_DIR}/core/commands/rep_cmd.cpp
    ${PROJECT_SOURCE_DIR}/core/mount/mount_manager.cpp
    ${PROJECT_SOURCE_DIR}/core/filesystem/ext2_writer.cpp
    ${PROJECT_SOURCE_DIR}/core/filesystem/ext2_reader.cpp
    ${PROJECT_SOURCE_DIR}/core/filesystem/login.cpp
    ${PROJECT_SOURCE_DIR}/core/filesystem/logout.cpp
    ${PROJECT_SOURCE_DIR}/core/filesystem/session_manager.cpp
    ${PROJECT_SOURCE_DIR}/core/disk/disk_creator.cpp
    ${PROJECT_SOURCE_DIR}/core/utils/parser.cpp
    ${PROJECT_SOURCE_DIR}/core/reports/report_mbr.cpp
    ${PROJECT_SOURCE_DIR}/core/reports/report_sb.cpp
    ${PROJECT_SOURCE_DIR}/core/reports/report_bm.cpp
    ${PROJECT_SOURCE_DIR}/core/reports/report_file.cpp
    ${PROJECT_SOURCE_DIR}/core/reports/report_disk.cpp
    ${PROJECT_SOURCE_DIR}/core/reports/report_inode.cpp
    ${PROJECT_SOURCE_DIR}/core/reports/report_block.cpp
    ${PROJECT_SOURCE_DIR}/core/reports/report_tree.cpp
    ${PROJECT_SOURCE_DIR}/core/reports/report_ls.cpp
    ${PROJECT_SOURCE_DIR}/core/server/server.cpp
)

add_executable(extreamfs ${SOURCES})
find_package(Threads REQUIRED)
target_link_libraries(extreamfs Threads::Threads)