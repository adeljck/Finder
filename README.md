## finder 使用说明

<p align="center">
  <img src="assets/logo.svg" alt="finder logo" width="420" />
</p>

![Finder](https://socialify.git.ci/adeljck/Finder/image?description=1&descriptionEditable=Windows后渗透工具&font=Bitter&forks=1&issues=1&language=1&logo=https://avatars.githubusercontent.com/u/24542600?v=4&name=1&owner=1&pattern=Circuit%20Board&pulls=1&stargazers=1&theme=Dark)


［简体中文］| [English](README_en.md)

一个在 Windows 上多线程扫描文件名的命令行工具，按关键词匹配文件/目录名并输出为 UTF-8 编码的 CSV。

### 工具原理

- NTFS/MFT：在 NTFS 分区上，目录枚举底层由 `$MFT` 的目录索引（B+ 树）支撑；本工具通过 Win32 文件枚举 API（如 `FindFirstFileW/FindNextFileW`）进行遍历，间接利用 NTFS 的索引结构实现高效列举，并未直接解析原始 `$MFT` 数据或裸盘扇区。


### 编译

- 使用 MSVC（x64 Native Tools Command Prompt 或 PowerShell 中调用 cl）：

```
./build.ps1
```

编译成功后会生成 `finder.exe`。

### 基本用法

```
finder.exe [选项] [路径1 路径2 ...]
```

- 若未指定任何路径且未使用 `-all`，程序会打印用法并退出。
- 指定 `-all` 时，程序会枚举所有可用盘符（固定盘/可移动盘）作为扫描起点。
- 输出为 CSV，每行包含：全路径、名称、命中关键词（均为 UTF-8 编码，无表头）。

### 选项

- `-t <N>`: 设置线程数。默认等于 CPU 逻辑核心数，至少为 1。
- `-o <文件名>`: 将结果写入指定文件（UTF-8，无 BOM）。不指定则输出到标准输出。
- `-norec`: 非递归模式。仅扫描给定目录本层，不进入子目录。
- `-followsymlink`: 递归时跟随符号链接/重解析点。默认不跟随以避免潜在循环。
- `-k <kw1,kw2,...>`: 逗号分隔的关键词列表（大小写不敏感）。
  - 未指定时，默认关键词：`vpn,password,passwd,pwd,account,账户,密码,账号,台账,服务器`。
- `-all`: 扫描所有可用盘符（固定盘/可移动盘），无需再指定路径。
- `-nonntfs`: 允许遍历非 NTFS 卷。默认跳过非 NTFS（FAT32/exFAT/网络盘等）根目录；指定此参数后将对这些卷执行同样的目录枚举。
- `--debug`: 控制台输出每条命中明细（默认仅输出总命中数）。
- `--debug-denied`: 当因权限不足无法进入目录时，在控制台输出被拒绝的目录路径。

说明：关键词匹配基于文件名的小写形式进行 `substring` 检查。例如关键词 `pwd` 会命中 `my_pwd_notes.txt`。

### 输出与日志

- 默认行为：
  - 不在控制台打印每条命中，仅在结束时打印总命中数，例如：`Total matches: 123`。
  - 若未使用 `-o` 指定输出文件名，会自动写入 `result_YYYYMMDD_HHMMSS.csv`（UTF-8，无 BOM）。
- 调试模式：
  - 使用 `--debug` 可同时在控制台打印每条命中明细（CSV 同格式）。

### CSV 格式

- 每行三个字段：
  1. `fullpath`（示例：`"C:\\Users\\Alice\\Desktop\\pwd.txt"`）
  2. `name`（示例：`"pwd.txt"`）
  3. `keyword`（示例：`"pwd"`）
- 字段使用双引号包裹，内部双引号采用 CSV 标准的“重复一个双引号”方式转义。例如：

```
"C:\\path\\with""quote"".txt","with""quote"".txt","password"
```

### 示例

- 扫描 D: 盘，关键词为中文“密码”与“账户”，结果输出到 `result.csv`：

```
finder.exe -k 密码,账户 -o result.csv D:\
```

- 多线程（8 线程）扫描两个目录，递归且不跟随符号链接，结果打印到控制台：

```
finder.exe -t 8 C:\\Users\\Alice\\Downloads C:\\Data
```

- 使用默认关键词，扫描所有可用盘符，输出到文件：

```
finder.exe -o found.csv
```

### 行为与性能

- 目录调度采用多线程工作队列；每发现子目录会入队继续扫描。
- 输出使用全局互斥保护，保证每条记录完整写出。
- 初次写文件时以截断模式打开，如需追加请先自行备份或改为重定向追加到目标文件。

### 回收站匹配（$I 元数据）

- 支持在回收站路径（如 `C:\$Recycle.Bin\<SID>\`，旧版为 `C:\RECYCLER\`/`C:\RECYCLED\`）内按“原始文件名”匹配。
- 实现方式：遇到 `$Ixxxxx.*` 元数据文件时，解析其中记录的原始完整路径与名称；若原始名称命中关键词，则按原始路径/名称输出对应命中记录。
- 限制：仅解析 Vista/Win7+ 的 `$I` 格式；不读取 `$R` 数据文件内容；跨用户 SID 目录通常受权限限制，未授权目录将被跳过（可用 `--debug-denied` 查看）。

### 注意事项

- 访问受限目录或离线/损坏盘符时，可能出现“无法打开目录”的情况；程序会跳过并继续。
- `-followsymlink` 可能导致循环遍历（取决于链接结构），默认关闭以避免走入递归陷阱。
- 采用 `std::wstring` 与 Windows API 扫描，路径与名称通过 UTF-16→UTF-8 转换后输出；某些极端字符在不同区域设置下的大小写折叠可能表现差异。

### 退出码

- `0`: 正常完成。
- 非 0：进程级异常或未处理的运行时错误（极少见）。

### 反馈

如需新增特性（如输出表头、追加模式、按扩展名过滤、忽略大小写规则配置等），请在 issue 中提出需求或直接提交 PR。

### 项目结构

```
.
├─ include/
│  ├─ app.h              // 运行入口声明
│  ├─ config.h           // 配置结构与参数解析接口
│  ├─ output.h           // 输出与计数接口
│  ├─ scanner.h          // 扫描器与工作队列接口
│  └─ utils.h            // 工具函数（路径、编码、时间戳等）
├─ src/
│  ├─ app.cpp            // 程序主流程（参数解析、启动扫描、结束汇总）
│  ├─ config.cpp         // 参数解析实现
│  ├─ output.cpp         // 结果输出与调试打印、计数
│  ├─ scanner.cpp        // 多线程扫描与队列调度
│  └─ utils.cpp          // 工具函数实现
├─ main.cpp              // 最小入口，调用 run_app
├─ build.ps1             // 一键构建脚本（去溯源、自动清理）
└─ README.md
```

### 隐私/去溯源编译（MSVC）

目标：可执行文件中不包含开发者机器路径、用户名、调试符号与构建时间戳等可溯源信息。

- 推荐 Release 构建且不生成调试符号：

```
./build.ps1
```

- 说明：
  - 不使用 `/Zi`（或任何生成 PDB 的选项），并在链接阶段指定 `/DEBUG:NONE /PDB:NONE`，避免产生或引用 PDB（其中会包含本机路径）。
  - `/Brepro`（编译与链接）启用可重现构建，去除时间戳等非确定性信息。
  - `/INCREMENTAL:NO` 禁增量链接；`/OPT:REF /OPT:ICF` 清理未用符号，减少泄露风险与体积。
  - 工具链签名（如 PE “Rich” Header）属于编译器自身信息，常规链接选项无法去除；若需彻底清除，需要额外的二进制后处理工具或改用不同工具链构建。


