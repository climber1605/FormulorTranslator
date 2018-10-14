# <center>FormulorTranslator</center>
数学公式排版语言：参照 MathJax 及 Latex

一、 符号集

小写字母：a..z，A..Z

数字：0..9

特殊符号：\、（、）、{、}、_、^、$

分隔符：空格、换行

二、 记号定义

标识符：由字母开始，由字母和数字组成，显示为斜体

数字：无符号整型数，显示为普通字体

三、 语言规则

1. 句子：
    $α$

2. 上下标：
    上标^{α}，下标_{α}，上下标同时有用_^{α}{β}其中 α 是下标，β 是上标。

3. 括号：
    仅使用()嵌套。

4. 空格：
    \blank

5. 大型运算：
    积分：\int{α}{β}{γ}
    累加：\sum{α}{β}{γ}
    其中 α 是下限，β 是上限，γ 是运算内容。

  四、 基础文法

 S -> $B$
  ​	B -> BB
  ​	B-> B_^{B}{B}
  ​	B -> B^{B}
  ​	B -> B_{B}
  ​	B- > \int{B}{B}{B}
  ​	B -> \sum{B}{B}{B}
  ​	B -> id | num | \blank | (B)  

  五、 输入输出

  输入：文本文件，一个输入文件中仅包含一个公式。
  样例文件文件名：sample??.txt， 测试文件文件名：test??.txt

  输出：超文本标记语言文件。
  样例文件文件名：sample??.html， 测试文件文件名：test??.html
