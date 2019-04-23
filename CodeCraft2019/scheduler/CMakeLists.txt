# 查找当前目录下的所有源文件
aux_source_directory(. DIR_SCHEDULER_SRCS)

# 指定生成目标
add_library(scheduler SHARED ${DIR_SCHEDULER_SRCS})
