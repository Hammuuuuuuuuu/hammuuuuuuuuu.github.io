/** allows the program to receive i/o */
const readline = require("readline");

// --- Stack Functions ---
function createStack() {
    return [];
}

function push(stack, x) {
    stack.push(x);
}

function pop(stack) {
    if (stack.length === 0) {
        throw new Error("Stack underflow");
    }
    return stack.pop();
}

function clearStack(stack) {
    stack.length = 0;
}

function peek(stack) {
    return stack[stack.length - 1];
}

function isStackEmpty(stack) {
    return stack.length === 0;
}

// --- Symbol Table Functions ---
function createSymbolTable(size = 26) {
    // Array of buckets, though for A-Z, direct indexing is possible.
    // We'll stick to the logic of bucket array for generic structure compliance,
    // but optimized for A-Z.
    return Array.from({ length: size }, () => []);
}

function hash(key) {
    return key.charCodeAt(0) - 65;
}

function validateKey(key) {
    if (typeof key !== "string" || key.length !== 1 || key < "A" || key > "Z") {
        throw new Error("Variable name must be a single letter A–Z");
    }
}

function insert(table, key, value) {
    validateKey(key);
    const index = hash(key);

    for (let entry of table[index]) {
        if (entry.key === key) {
            entry.value = value;
            return;
        }
    }

    table[index].push({ key, value });
}

function search(table, key) {
    validateKey(key);
    const index = hash(key);

    for (let entry of table[index]) {
        if (entry.key === key) {
            return entry.value;
        }
    }

    throw new Error(`Variable ${key} not found`);
}

function remove(table, key) {
    validateKey(key);
    const index = hash(key);
    table[index] = table[index].filter(e => e.key !== key);
}

// --- Interpreter Functions ---

function isBinaryOperator(token) {
    return ["+", "-", "*", "/", "%", "^"].includes(token);
}

function isUnaryOperator(token) {
    return ["sqrt", "cube", "log"].includes(token);
}

function resolveValue(val, table) {
    if (typeof val === "string") {
        return search(table, val);
    }
    return val;
}

function evaluate(line, stack, table) {
    try {
        const tokens = line.trim().split(/\s+/);

        for (let token of tokens) {
            if (!token) continue;

            if (!isNaN(token)) {
                // Parse float to support sqrt, log, etc.
                push(stack, parseFloat(token));
            }
            else if (token.length === 1 && token >= "A" && token <= "Z") {
                // Push variable name
                push(stack, token);
            }
            else if (isBinaryOperator(token)) {
                let b = pop(stack);
                let a = pop(stack);

                a = resolveValue(a, table);
                b = resolveValue(b, table);

                let res;
                switch (token) {
                    case "+": res = a + b; break;
                    case "-": res = a - b; break;
                    case "*": res = a * b; break;
                    case "/": res = a / b; break; // Floating point division
                    case "%": res = a % b; break;
                    case "^": res = Math.pow(a, b); break;
                }
                push(stack, res);
            }
            else if (isUnaryOperator(token)) {
                let a = pop(stack);
                a = resolveValue(a, table);

                let res;
                switch (token) {
                    case "sqrt": res = Math.sqrt(a); break;
                    case "cube": res = Math.pow(a, 3); break;
                    case "log": res = Math.log(a); break; // Natural log
                }
                push(stack, res);
            }
            else if (token === "=") {
                const value = pop(stack);
                const variable = pop(stack);

                if (typeof variable !== "string") {
                    throw new Error("Assignment requires a variable name");
                }

                // Value might be a variable name too? E.g. A B =
                const resolvedValue = resolveValue(value, table);

                insert(table, variable, resolvedValue);
                console.log(`${variable} = ${resolvedValue}`);
            }
            else {
                throw new Error(`Invalid token: ${token}`);
            }
        }

        if (!isStackEmpty(stack)) {
            let result = peek(stack);
            result = resolveValue(result, table);
            console.log("Result:", result);
            return result; // Return for testing
        }

    } catch (error) {
        console.log("Error:", error.message);
        throw error; // Re-throw for testing if needed, but in CLI loop it's caught
    } finally {
        clearStack(stack);
    }
}

// --- Main Execution ---

// Initialize shared state
const globalStack = createStack();
const globalSymbolTable = createSymbolTable();

function showMenu() {
    console.log("\nWelcome to Postfix++ Calculator Menu");
    console.log("1. INSERT");
    console.log("2. SEARCH");
    console.log("3. DELETE");
    console.log("4. EXIT");
}

function promptUser(rl) {
    showMenu();
    rl.question("Choose an option: ", choice => {
        try {
            if (choice === "1") {
                rl.question("Enter Postfix expression: ", expr => {
                    // We catch here to not crash the loop
                    try {
                        evaluate(expr, globalStack, globalSymbolTable);
                    } catch (e) {
                         // Error already logged in evaluate usually, or re-thrown
                    }
                    promptUser(rl);
                });
            } else if (choice === "2") {
                rl.question("Enter variable name: ", v => {
                    try {
                        console.log("Value:", search(globalSymbolTable, v));
                    } catch (e) {
                        console.log("Error:", e.message);
                    }
                    promptUser(rl);
                });
            } else if (choice === "3") {
                rl.question("Enter variable name to delete: ", v => {
                    try {
                        remove(globalSymbolTable, v);
                        console.log("Variable deleted");
                    } catch (e) {
                        console.log("Error:", e.message);
                    }
                    promptUser(rl);
                });
            } else if (choice === "4") {
                console.log("Exiting...");
                rl.close();
            } else {
                console.log("Invalid choice");
                promptUser(rl);
            }
        } catch (e) {
            console.log("Error:", e.message);
            promptUser(rl);
        }
    });
}

function main() {
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout
    });
    promptUser(rl);
}

// Only run main if this file is executed directly
if (require.main === module) {
    main();
}

// Export functions for testing
module.exports = {
    evaluate,
    createStack,
    createSymbolTable,
    insert,
    search,
    remove,
    push,
    pop,
    peek,
    clearStack,
    isStackEmpty,
    hash
};
