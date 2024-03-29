                  /$$$$$$         /$$$$$$         /$$$$$$         /$$$$$$ 
                 /$$__  $$       /$$__  $$       /$$__  $$       /$$__  $$
                | $$  \__/      | $$  \__/      | $$  \ $$      | $$  \__/
                | $$            | $$            |  $$$$$$/      | $$      
                | $$            | $$             >$$__  $$      | $$      
                | $$    $$      | $$    $$      | $$  \ $$      | $$    $$
                |  $$$$$$/      |  $$$$$$/      |  $$$$$$/      |  $$$$$$/
                 \______/        \______/        \______/        \______/
                 

# Chocolate Chip 8 Cookie
A CHIP-8 emulator built in C with SDL.

| Splash Screen | Breakout Screenshot |
|:--------:|:-----------------:|
|![splash_screen](./media/splash_screen.png)|![breakout](./media/breakout.png)|

| Console Output | Debug Output |
|:--------:|:-----------------:|
|![console_output](./media/console_output.png)|![debug_example](./media/debug_example.png)|

### Dependencies
- GCC
- SDL2
- Make
- Compatible CHIP8 ROM

### Build
```
cd src
make
```

### Run
```
./cc8c [Path to ROM] (Optional flag for debugging '-d')
```
Further instructions will be printed out once the emulator is run or you may also view ./src/instructions.txt as well.
