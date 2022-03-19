# XSym

Modeling language-specific built-in functions is essential for symbolic execution on the web. Existing solutions that manually translate these built-in functions into the SMT-LIB format are labor-intensive and error-prone. XSym is a research prototype exploring the feasibility of automated PHP built-in function modeling for PHP symbolic execution. It first synthesizes C programs by transforming the constraint solving task in PHP symbolic execution into a C-compliant format and integrating them with the C implementations of the built-in functions. After that, XSym employs symbolic execution on the synthesized C program to find a solution that can satisfy the original PHP constraints. 

XSym has been tested on Debian GNU/Linux 9.12. 

## Build

Use [composer](https://getcomposer.org/) to install the dependencies specified in `composer.json`. 

```sh
cd src/php_se
composer install
```

XSym currently uses [KLEE](https://github.com/klee/klee) for its C symbolic execution. Our modified KLEE is at `src/klee`. You are kindly suggested to refer to [KLEE's documents](http://klee.github.io/docs/) for installation instructions.

## Run

### PHP Symbolic Execution

XSym analyzes the PHP source code, constructs control-flow graphs and call graphs, and outputs the PHP constraint solving tasks.

```sh
cd src/php-se
# to analyze a single php script, use php Main.php [script]
php Main.php app.php
# to analyze a whole application, use php Main.php [app-directory]
php Main.php app/
```

This stage outputs constrainnts written as PHP code. Note that XSym currently cannot support some complex PHP features like nested array access, etc.

### C-Compliant Program Synthesis

XSym synthesizes C-compliant programs for PHP constraint solving tasks. It adds checkpoints in the synthesized C programs, which are triggered when the PHP constraint solving tasks are solved.

```sh
cd src/php-se
# convert the task in task.php to a C program in main.c
php Synth.php [XSS.txt] [XSSData.txt]
```

### C Symbolic Execution

Compile the C-compliant programs.

```sh
cd src/php-src
./compile.sh [main.c] [target]
```

Leverage KLEE to trigger the checkpoints. 

```sh
cd src/klee
./runklee.sh [target]
```

You can check the [documents](https://klee.github.io/docs) of KLEE for more instructions.

## License

XSym is under [MIT License](LICENSE).

## Publication

You can find more details in our [WWW 2021 paper](https://seclab.cse.cuhk.edu.hk/papers/www21_xsym.pdf).

```tex
@inproceedings{li2021xsym,
    title = {On the Feasibility of Automated Built-in Function Modeling for PHP Symbolic Execution},
    author = {Li, Penghui and Meng, Wei and Lu, Kangjie and Luo, Changhua},
    booktitle = {Proceedings of the Web Conference 2021},
    isbn = {9781450383127},
    url = {https://doi.org/10.1145/3442381.3450002},
    doi = {10.1145/3442381.3450002},
    month = {apr},
    year = {2021}
    location = {Ljubljana, Slovenia},
}
```

## Contacts

- Penghui Li (<phli@cse.cuhk.edu.hk>)
- Wei Meng (<wei@cse.cuhk.edu.hk>)
- Kangjie Lu (<kjlu@umn.edu>)
- Changhua Luo (<chluo@cse.cuhk.edu.hk>)
