项目名称 EZEngine
本项目旨在尝试将引擎拆解为程序资产来实现解耦合

文件夹功能分类

engine ：未来负责存储引擎模板配置的文件夹
module ：存储引擎程序资产的文件夹
Project：存储项目文件的文件夹
src ：存放抽象接口，封装基础类型的文件夹
vector : 暂时用于存放第三方库
编译与运行
尚未接入cmake，无法直接编译与运行

项目结构
|- engine
|- module/.. ## 该文件夹中有 README 注解
|- Project/..
|- src/..
|- vector/..
