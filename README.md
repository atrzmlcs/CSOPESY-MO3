### Made by CSOPESY S04 Group 2:
- Dumaran
- Malicsi
- Marinas

To compile, run the following command while in the directory:
`g++ -g main.cpp marquee_console.cpp -o main.exe`

To run using Visual Studio Code, go to your directory's generated .vscode folder, go into tasks.json, and change "args" into the following:
```
      "args": [
        "-fdiagnostics-color=always",
        "-g",
        "main.cpp",
        "marquee_console.cpp",
        "-o",
        "${fileDirname}\\${fileBasenameNoExtension}.exe"
      ],
```
