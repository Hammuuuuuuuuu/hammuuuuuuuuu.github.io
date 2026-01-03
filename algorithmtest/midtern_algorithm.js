/** allows the program to receive i/o */
const readline = require("readline");

class Stack {
    constructor() {
        this.items = [];
    }

    push(x) {
        this.items.unshift(x);
    }

    pop() {
        if (this.items.length === 0) {
            throw new Error("Stack underflow");
        }
        return this.items.shift();
    }

    clear() {
        this.items = [];
    }

    peek() {
        return this.items[0];
    }

    isEmpty() {
        return this.items.length === 0;
    }
}

class SymbolTable {
    constructor(size = 26) {
        this.size = size;
        this.table = Array.from({ length: size }, () => []);
    }

    hash(key) {
        return key.charCodeAt(0) - 65;
    }

    insert(key, value) {
        if (typeof key !== "string" || key.length !== 1 || key < "A" || key > "Z") {
            throw new Error("Variable name must be a single letter A–Z");
        }

        const index = this.hash(key);

        for (let entry of this.table[index]) {
            if (entry.key === key) {
                entry.value = value;
                return;
            }
        }

        this.table[index].push({ key, value });
    }


        search(key) {
        if (typeof key !== "string" || key.length !== 1 || key < "A" || key > "Z") {
            throw new Error("Variable name must be a single letter A–Z");
        }

        const index = this.hash(key);

        for (let entry of this.table[index]) {
            if (entry.key === key) {
                return entry.value;
            }
        }

        throw new Error(`Variable ${key} not found`);
    }

    delete(key) {
        if (typeof key !== "string" || key.length !== 1 || key < "A" || key > "Z") {
            throw new Error("Variable name must be a single letter A–Z");
        }

        const index = this.hash(key);
        this.table[index] = this.table[index].filter(e => e.key !== key);
    }
}

class PostfixInterpreter {
    constructor() {
        this.stack = new Stack();
        this.symbolTable = new SymbolTable();
    }

    isOperator(token) {
        return ["+", "-", "*", "/"].includes(token);
    }

    applyOperator(op, a, b) {
        switch (op) {
            case "+": return a + b;
            case "-": return a - b;
            case "*": return a * b;
            case "/": return Math.floor(a / b);
        }
    }

    evaluate(line) {
    try {
        const tokens = line.trim().split(/\s+/);

        for (let token of tokens) {
            if (!isNaN(token)) {
                this.stack.push(parseInt(token));
            } 
            else if (token >= "A" && token <= "Z") {
                // Push variable name for now
                this.stack.push(token);
            } 
            else if (this.isOperator(token)) {
                let b = this.stack.pop();
                let a = this.stack.pop();

                if (typeof a === "string") {
                    a = this.symbolTable.search(a);
                }
                if (typeof b === "string") {
                    b = this.symbolTable.search(b);
                }

                this.stack.push(this.applyOperator(token, a, b));
            } 
            else if (token === "=") {
                const value = this.stack.pop();
                const variable = this.stack.pop();

                if (typeof variable !== "string") {
                    throw new Error("Assignment requires a variable name");
                }

                this.symbolTable.insert(variable, value);
                console.log(`${variable} = ${value}`);
            }
            else {
                throw new Error(`Invalid token: ${token}`);
            }
        }

        if (!this.stack.isEmpty()) {
            let result = this.stack.peek();
            if (typeof result === "string") {
                result = this.symbolTable.search(result);
            }
            console.log("Result:", result);
        }


    } catch (error) {
        console.log("Error:", error.message);
    } finally {
        // Always clear stack after each line
        this.stack.clear();
        }   
    }
}

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
});

const interpreter = new PostfixInterpreter();

function showMenu() {
    console.log("\nWelcome to Postfix++ Calculator Menu");
    console.log("1. INSERT");
    console.log("2. SEARCH");
    console.log("3. DELETE");
    console.log("4. EXIT");
}

function promptUser() {
    showMenu();
    rl.question("Choose an option: ", choice => {
        try {
            if (choice === "1") {
            rl.question("Enter Postfix expression: ", expr => {
                interpreter.evaluate(expr);
                promptUser();
            });
            } else if (choice === "2") {
                rl.question("Enter variable name: ", v => {
                    try {
                        console.log("Value:", interpreter.symbolTable.search(v));
                    } catch (e) {
                        console.log("Error:", e.message);
                    }
                    promptUser();
                });
            } else if (choice === "3") {
                rl.question("Enter variable name to delete: ", v => {
                    try {
                        interpreter.symbolTable.delete(v);
                        console.log("Variable deleted");
                    } catch (e) {
                        console.log("Error:", e.message);
                    }
                    promptUser();
                });
            } else if (choice === "4") {
                console.log("Exiting...");
                rl.close();
            } else {
                console.log("Invalid choice");
                promptUser();
            }
        } catch (e) {
            console.log("Error:", e.message);
            promptUser();
        }
    });
}

promptUser();