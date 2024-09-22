const child_process = require("child_process")

var lastTime = undefined

for (var i = 0; i < 10; i++) {
    child_process.execSync(`g++ -DARRAY_SIZE=${ (1 << i) + 1 } -pthread -o main main.cpp`)
    const time = parseInt(child_process.execSync(`./main`).toString())
    if (lastTime && time < lastTime / 2) {
        console.log(`Measured cache line is ${ (1 << i) } bytes`)
        return
    }
    lastTime = time
}
