#include<iostream>
#include<cmath>
#include<string>
using namespace std;

#define max(a,b) ((a)>(b) ? (a):(b))
#define MAX 200		//词法记号数最大值
#define NONE 0x300   //初始词法记号名
#define INIT_FONTSIZE 80	//初始fontsize属性值，即默认字体大小
#define INIT_LEFT 100	//初始left属性值
#define INIT_TOP 100	//初始top属性值
#define FONT_FAMILY "Monaco"	//字体种类
const double leftOffset = 0.6;	//用于计算下标的left属性值的常量
const double topOffset = 0.6;	//用于计算下标的top属性值的常量
const double fontsizeShrink = 0.5;	//用于计算上下标的font-size属性值的常量

									//定义分析状态
#define SUCCEED 0
#define SPELL_ERROR -1
#define UNDEFINED_SYMBOL -2
#define INCOMPLETE -3
#define MISMATCH -4
#define UNDEFINED_ITEM -5
#define OPEN_FILE_FAIL -6
#define UNKNOWN_MISTAKE  -7

									//定义词法记号名
const int id = 0x100;		//标识符
const int num = 0x101;		//无符号整数
const int blank = 0x102;	//空格符
const int intergral = 0x103;	//积分号
const int sum = 0x104;		//求和号
const int fullScript = 0x105;	//上下标
const int superScript = 0x106;	//上标
const int subScript = 0x107;	//下标
const int lParenthese = 0x108;	//左圆括号
const int rParenthese = 0x109;	//右圆括号
const int lBrace = 0x10A;	//左花括号
const int rBrace = 0x10B;	//右花括号
const int dollar = 0x10C;	//$符

							//将整型的记号名映射成字符串
string marks[] = { "id","num","blank","\\int","\\sum","_^","^","_","(",")","{","}","$" };

struct tokenType {
	int name;	//记号名
	string value;	//属性值
}tokens[MAX];		//tokens用于存放词法分析器扫描文件后得到的记号流

tokenType *pToken = tokens;		//输入串指针
tokenType token;	//当前输入串指针所指向的记号
FILE *fHtml;	//输出文件指针

int scanner(FILE *fin);		//词法分析器，扫描输入文件将所得记号流存放在tokens数组中
int fail(int start);		//将当前状态置为下一个状态转换图的开始状态，准备匹配下一状态转换图
void parser(void);			//语法分析解释器，使用递归下降的预测分析方法，边分析边解释
void S(int left, int top, int fontSize);
void B(int &left, int top, int fontSize, bool fontStyle);
void F(int &left, int top, int fontSize);
void match(int t);			//判断当前记号是否与期望的记号t匹配
bool isFirstB(int t);		//判断终结符t是否属于FIRST(B)
bool isFollowF(int t);		//判断终结符t是否属于FOLLOW(F)
							//向html文件输出一个符号
void printToken(int left, int top, int fontsize, bool fontstyle, string tokenVal);
int main(int argc, char * argv[]) {
	FILE *fin, *fout;
	char ch;
	if (argc == 1) {
		cout << "请输入句子（以crtl+Z结束）" << endl;
		if ((fout = fopen("sentence.in", "w")) == NULL) {
			return OPEN_FILE_FAIL;
		}
		while ((ch = getchar()) != EOF) {
			fputc(ch, fout);
		}
		fclose(fout);
		cout << endl;
	}
	else {
		if ((fin = fopen(argv[1], "r")) == NULL) {
			return OPEN_FILE_FAIL;
		}
		if ((fout = fopen("sentence.in", "w")) == NULL) {
			return OPEN_FILE_FAIL;
		}
		while ((ch = fgetc(fin)) != EOF) {
			fputc(ch, fout);
		}
		cout << endl;
		fclose(fin);
		fclose(fout);
	}

	if ((fin = fopen("sentence.in", "r")) == NULL) {
		return OPEN_FILE_FAIL;
	}
	if ((fHtml = fopen("result.html", "w")) == NULL) {
		return OPEN_FILE_FAIL;
	}
	fprintf(fHtml, "<html>\n");
	fprintf(fHtml, "\t<head>\n");
	fprintf(fHtml, "\t\t<meta http-equiv=\"Content-type\" content=\"text/html;charset=utf-8\" 		/>\n");
	fprintf(fHtml, "\t</head>\n");
	fprintf(fHtml, "\t<body>\n");

	cout << "-------------------------Lexical Analysis-------------------------" << endl << endl;
	for (int i = 0; i < MAX; i++) {
		tokens[i].name = NONE;
		tokens[i].value = "";
	}
	int scannerState = scanner(fin);
	cout << endl;
	switch (scannerState) {
	case SUCCEED:
		cout << "Lexical Analysis finishes with no error detected" << endl << endl;
		break;
	case SPELL_ERROR:
		cout << "-------------------------------------------------------------------" << endl;
		cout << "Translation stops unexpectedly due to spell error detected" << endl;
		return SPELL_ERROR;
	case UNDEFINED_SYMBOL:
		cout << "-------------------------------------------------------------------" << endl;
		cout << "Translation stops unexpectedly due to unknown symbol detected" << endl;
		return UNDEFINED_SYMBOL;
	default:
		cout << "-------------------------------------------------------------------" << endl;
		cout << "Translation stops unexpectedly due to unknown mistake" << endl;
		return UNKNOWN_MISTAKE;
	}

	cout << "-------------------------Syntax Analysis -------------------------" << endl << endl;
	parser();
	cout << endl << "Syntax Analysis finishes with no error detected" << endl << endl;

	cout << "-------------------------Translate to html -------------------------" << endl << endl;
	fprintf(fHtml, "\t</body>\n");
	fprintf(fHtml, "</html>\n");
	cout << "Translation finishes  with no error dected" << endl;

	fclose(fin);
	fclose(fHtml);
	return SUCCEED;
}

//词法分析器，扫描输入文件将所得记号流存放在tokens数组中
int scanner(FILE *fin) {
	int state = 0;	//DFA初始状态为0
	int i = 0;	//词法单元指针
	char ch = '0';
	while (!feof(fin)) {
		switch (state) {
		case 0:
			ch = fgetc(fin);
			if (ch == EOF) {
				return SUCCEED;
			}
			if (isspace(ch)) {
				state = 0;
			}
			else if (isalpha(ch)) {
				tokens[i].value += ch;
				state = 1;
			}
			else state = fail(state);
			break;
		case 1:
			ch = fgetc(fin);
			if (isalpha(ch) || isdigit(ch)) {
				tokens[i].value += ch;
				state = 1;
			}
			else {
				state = 2;
			}
			break;
		case 2:
			fseek(fin, -1, SEEK_CUR);
			tokens[i].name = id;	//识别出记号id
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//准备识别下一记号
			break;
		case 3:
			if (isdigit(ch)) {
				tokens[i].value += ch;
				state = 4;
			}
			else {
				state = fail(state);
			}
			break;
		case 4:
			ch = fgetc(fin);
			if (isdigit(ch)) {
				tokens[i].value += ch;
				state = 4;
			}
			else {
				state = 5;
			}
			break;
		case 5:
			fseek(fin, -1, SEEK_CUR);
			tokens[i].name = num;	//识别出记号num
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//准备识别下一记号
			break;
		case 6:
			if (ch == '\\') {
				tokens[i].value += ch;
				state = 7;
			}
			else {
				state = fail(state);
			}
			break;
		case 7:
			ch = fgetc(fin);
			if (ch == 'b') {
				tokens[i].value += ch;
				state = 8;
			}
			else if (ch == 'i') {
				tokens[i].value += ch;
				state = 13;
			}
			else if (ch == 's') {
				tokens[i].value += ch;
				state = 16;
			}
			else {
				cout << endl << '\t' << '\\' << ch << endl;
				cout << "Lexical error: invalid token,maybe you spell a keyword wrongly" << endl;
				return SPELL_ERROR;
			}
			break;
		case 8:
			ch = fgetc(fin);
			if (ch == 'l') {
				tokens[i].value += ch;
				state = 9;
			}
			else {
				cout << endl << '\t' << "\\b" << ch << endl;
				cout << "Lexical error: invalid token,maybe you spell blank wrongly,try to change " << ch << " to l" << endl;
				return SPELL_ERROR;
			}
			break;
		case 9:
			ch = fgetc(fin);
			if (ch == 'a') {
				tokens[i].value += ch;
				state = 10;
			}
			else {
				cout << endl << '\t' << "\\bl" << ch << endl;
				cout << "Lexical error: invalid token,maybe you spell blank wrongly,try to change " << ch << " to a" << endl;
				return SPELL_ERROR;
			}
			break;
		case 10:
			ch = fgetc(fin);
			if (ch == 'n') {
				tokens[i].value += ch;
				state = 11;
			}
			else {
				cout << endl << '\t' << "\\bla" << ch << endl;
				cout << "Lexical error: invalid token,maybe you spell blank wrongly,try to change " << ch << " to n" << endl;
				return SPELL_ERROR;
			}
			break;
		case 11:
			ch = fgetc(fin);
			if (ch == 'k') {
				tokens[i].value += ch;
				state = 12;
			}
			else {
				cout << endl << '\t' << "\\blan" << ch << endl;
				cout << "Lexical error: invalid token,maybe you spell blank wrongly,try to change " << ch << " to k" << endl;
				return SPELL_ERROR;
			}
			break;
		case 12:
			tokens[i].name = blank;	//识别出记号\blank
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//准备识别下一记号
			break;
		case 13:
			ch = fgetc(fin);
			if (ch == 'n') {
				tokens[i].value += ch;
				state = 14;
			}
			else {
				cout << endl << '\t' << "\\i" << ch << endl;
				cout << "Lexical error: invalid token,maybe you spell int wrongly,try to change " << ch << " to n" << endl;
				return SPELL_ERROR;
			}
			break;
		case 14:
			ch = fgetc(fin);
			if (ch == 't') {
				tokens[i].value += ch;
				state = 15;
			}
			else {
				cout << endl << '\t' << "\\in" << ch << endl;
				cout << "Lexical error: invalid token,maybe you spell int wrongly,try to change " << ch << " to t" << endl;
				return SPELL_ERROR;
			}
			break;
		case 15:
			tokens[i].name = intergral;	//识别出记号\int
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//准备识别下一记号
			break;
		case 16:
			ch = fgetc(fin);
			if (ch == 'u') {
				tokens[i].value += ch;
				state = 17;
			}
			else {
				cout << endl << '\t' << "\\s" << ch << endl;
				cout << "Lexical error: invalid token,maybe you spell sum wrongly,try to change " << ch << " to u" << endl;
				return SPELL_ERROR;
			}
			break;
		case 17:
			ch = fgetc(fin);
			if (ch == 'm') {
				tokens[i].value += ch;
				state = 18;
			}
			else {
				cout << endl << '\t' << "\\su" << ch << endl;
				cout << "Lexical error: invalid token,maybe you spell sum wrongly,try to change " << ch << " to m" << endl;
				return SPELL_ERROR;
			}
			break;
		case 18:
			tokens[i].name = sum;	//识别出记号\sum
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//准备识别下一记号
			break;
		case 19:
			if (ch == '_') {
				tokens[i].value += ch;
				state = 20;
			}
			else if (ch == '^') {
				tokens[i].value += ch;
				state = 23;
			}
			else if (ch == '(') {
				tokens[i].value += ch;
				state = 24;
			}
			else if (ch == ')') {
				tokens[i].value += ch;
				state = 25;
			}
			else if (ch == '{') {
				tokens[i].value += ch;
				state = 26;
			}
			else if (ch == '}') {
				tokens[i].value += ch;
				state = 27;
			}
			else if (ch == '$') {
				tokens[i].value += ch;
				state = 28;
			}
			else {
				state = fail(state);
			}
			break;
		case 20:
			ch = fgetc(fin);
			if (ch == '^') {
				tokens[i].value += ch;
				state = 21;
			}
			else {
				state = 22;
			}
			break;
		case 21:
			tokens[i].name = fullScript;	//识别出记号_^
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//准备识别下一记号
			break;
		case 22:
			fseek(fin, -1, SEEK_CUR);
			tokens[i].name = subScript;	//识别出记号_
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//准备识别下一记号
			break;
		case 23:
			tokens[i].name = superScript;	//识别出记号^
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//准备识别下一记号
			break;
		case 24:
			tokens[i].name = lParenthese;	//识别出记号(
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//准备识别下一记号
			break;
		case 25:
			tokens[i].name = rParenthese;	//识别出记号)
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//准备识别下一记号
			break;
		case 26:
			tokens[i].name = lBrace;	//识别出记号{
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//准备识别下一记号
			break;
		case 27:
			tokens[i].name = rBrace;	//识别出记号}
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//准备识别下一记号
			break;
		case 28:
			tokens[i].name = dollar;	//识别出记号$
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//准备识别下一记号
			break;
		case NONE:
			cout << endl << '\t';
			for (int j = 0; j < i; j++) {
				cout << tokens[j].value;
			}
			cout << ch << endl;
			cout << "Lexical error: invalid token,unknown symbol " << ch << " detected,please check your input" << endl;
			return UNDEFINED_SYMBOL;
		default:
			return UNKNOWN_MISTAKE;
		}
	}
	return SUCCEED;
}

//将当前状态置为下一个状态转换图的开始状态，准备匹配下一个状态转换图
int fail(int start) {
	switch (start) {
	case 0:
		start = 3;
		break;
	case 3:
		start = 6;
		break;
	case 6:
		start = 19;
		break;
	case 19:
		return NONE;
	default:
		exit(UNKNOWN_MISTAKE);
	}
	return start;
}

//语法分析解释器，使用递归下降的预测分析方法，边分析边计算，所需记号已经全部存放在tokens数组中
void parser(void) {
	token = tokens[0];
	while (token.name != NONE) {
		int left = INIT_LEFT, top = INIT_TOP, fontSize = INIT_FONTSIZE;
		S(left, top, fontSize);
	}
}

void S(int left, int top, int fontSize) {
	if (token.name == dollar) {		//产生式S-->$BF$
		cout << "S-->$BF$" << endl;
		match(dollar);
		B(left, top, fontSize, false);
		F(left, top, fontSize);
		match(dollar);
	}
	else {	//语法错误
		cout << endl << '\t' << token.value << endl;
		cout << "Syntax error: missing a $ at the beginning,the sentence must begin with a $" << endl << endl;
		cout << "-------------------------------------------------------------------" << endl << endl;
		cout << "Translation stops unexpectedly due to unknown grammar production" << endl;
		exit(UNDEFINED_ITEM);
	}
}

void B(int &left, int top, int fontSize, bool fontStyle) {
	if (token.name == id) {		//产生式B-->id
		cout << "B-->id" << endl;
		fontStyle = true;
		printToken(left, top, fontSize, fontStyle, token.value);
		left += static_cast<int>(floor(fontSize*token.value.length() * leftOffset));
		match(id);
	}
	else if (token.name == num) {	//产生式B-->num
		cout << "B-->num" << endl;
		printToken(left, top, fontSize, fontStyle, token.value);
		left += static_cast<int>(floor(fontSize*token.value.length() * leftOffset));
		match(num);
	}
	else if (token.name == blank) {		//产生式B-->\blank
		cout << "B-->\\blank" << endl;
		printToken(left, top, fontSize, fontStyle, " ");
		left += static_cast<int>(floor(fontSize * leftOffset));
		match(blank);
	}
	else if (token.name == lParenthese) {	//产生式B-->(BF)
		cout << "B-->(BF)" << endl;
		printToken(left, top, fontSize, fontStyle, "(");
		match(lParenthese);
		left += static_cast<int>(floor(fontSize * leftOffset));
		B(left, top, fontSize, fontStyle);
		F(left, top, fontSize);
		printToken(left, top, fontSize, fontStyle, ")");
		left += static_cast<int>(floor(fontSize * leftOffset));
		match(rParenthese);
	}
	else if (token.name == intergral) {		//产生式B-->\int{BF}{BF}{BF}
		cout << "B-->\\int{BF}{BF}{BF}" << endl;
		int newFontSize = static_cast<int>(floor(fontSize * 1.1));
		printToken(left, top, newFontSize, fontStyle, "&int;");
		left += static_cast<int>(floor(newFontSize * 0.3));
		match(intergral);
		//匹配下标
		match(lBrace);
		int curLeft = left;	//暂时保存left属性，作为后续输出上标时的left属性
		int newTop = top + static_cast<int>(floor(fontSize * 0.7));
		newFontSize = static_cast<int>(floor(fontSize * fontsizeShrink));
		B(left, newTop, newFontSize, fontStyle);

		F(left, newTop, newFontSize);
		match(rBrace);
		//匹配上标
		match(lBrace);
		newTop = top - static_cast<int>(floor(fontSize * 0.3));
		newFontSize = static_cast<int>(floor(fontSize * fontsizeShrink));
		B(curLeft, newTop, newFontSize, fontStyle);
		F(curLeft, newTop, newFontSize);
		match(rBrace);
		//匹配求和内容
		match(lBrace);
		left = max(left, curLeft);
		B(left, top, fontSize, fontStyle);
		F(left, top, fontSize);
		match(rBrace);
	}
	else if (token.name == sum) {		//产生式B-->\sum{BF}{BF}{BF}
		cout << "B-->\\sum{BF}{BF}{BF}" << endl;
		int newFontSize = static_cast<int>(floor(fontSize * 1.1));
		printToken(left, top, newFontSize, fontStyle, "&Sigma;");
		left += static_cast<int>(floor(fontSize * 0.75));
		match(sum);
		//匹配下标
		match(lBrace);
		int curLeft = left;	//暂时保存left属性，作为后续输出上标时的left属性
		int newTop = top + static_cast<int>(floor(fontSize * topOffset));
		newFontSize = static_cast<int>(floor(fontSize * fontsizeShrink));
		B(left, newTop, newFontSize, fontStyle);
		F(left, newTop, newFontSize);
		match(rBrace);
		//匹配上标
		match(lBrace);
		B(curLeft, top, newFontSize, fontStyle);
		F(curLeft, top, newFontSize);

		match(rBrace);
		//匹配求和内容
		match(lBrace);
		left = max(left, curLeft);
		B(left, top, fontSize, fontStyle);
		F(left, top, fontSize);
		match(rBrace);
	}
	else {		//语法错误
		cout << endl << '\t' << (pToken - 1)->value << pToken->value << endl;
		cout << endl << "Syntax error: expect id,num,\\blank,(,\\int or \\sum while input is " << marks[token.name - id] << endl << endl;
		cout << "-------------------------------------------------------------------" << endl << endl;
		cout << "Translation stops unexpectedly due to unknown grammar production" << endl;
		exit(UNDEFINED_ITEM);
	}
}

void F(int &left, int top, int fontSize) {
	if (token.name == fullScript) {		//产生式F-->_^{BF}{BF}F
		cout << "F-->_^{BF}{BF}F" << endl;
		match(fullScript);
		//匹配下标
		match(lBrace);
		int curLeft = left;	//暂时保存left属性，作为后续输出上标时的left属性
		int newTop = top + static_cast<int>(floor(fontSize * topOffset));
		int newFontSize = static_cast<int>(floor(fontSize * fontsizeShrink));
		B(left, newTop, newFontSize, false);
		F(left, newTop, newFontSize);
		match(rBrace);
		//匹配上标
		match(lBrace);
		B(curLeft, top, newFontSize, false);
		F(curLeft, top, newFontSize);
		match(rBrace);
		//匹配剩余部分
		left = max(left, curLeft);
		F(left, top, fontSize);
	}
	else if (token.name == superScript) {	//产生式F-->^{BF}F
		cout << "F-->^{BF}F" << endl;
		match(superScript);
		//匹配上标
		match(lBrace);
		int newFontSize = static_cast<int>(floor(fontSize * fontsizeShrink));
		B(left, top, newFontSize, false);
		F(left, top, newFontSize);
		match(rBrace);
		//匹配剩余部分
		F(left, top, fontSize);
	}
	else if (token.name == subScript) {		//产生式F-->_{BF}F
		cout << "F-->_{BF}F" << endl;
		match(subScript);
		//匹配下标
		match(lBrace);
		int newTop = top + static_cast<int>(floor(fontSize * topOffset));
		int newFontSize = static_cast<int>(floor(fontSize * fontsizeShrink));
		B(left, newTop, newFontSize, false);
		F(left, newTop, newFontSize);
		match(rBrace);
		//匹配剩余部分
		F(left, top, fontSize);
	}
	else if (isFirstB(token.name)) {		//产生式F-->BF
		cout << "F-->BF" << endl;
		B(left, top, fontSize,false);
		F(left, top, fontSize);
	}
	else if (isFollowF(token.name) || token.name == NONE) {		//产生式F-->NULL
		cout << "F--> " << endl;
	}
	else {	//语法错误
		cout << endl << '\t' << (pToken - 1)->value << pToken->value << endl;
		cout << endl << "Syntax error: expect id,num,\\blank,(,),},_^,_,^,\\int,\\sum or $ while input is " << marks[token.name - id] << endl << endl;
		cout << "-------------------------------------------------------------------" << endl << endl;
		cout << "Translation stops unexpectedly due to unknown grammar production" << endl;
		exit(UNDEFINED_ITEM);
	}
}

//判断当前记号是否与期望的记号t匹配
void match(int t) {
	if (token.name == t) {
		token = *(++pToken);
	}
	else {
		if (t == dollar) {
			cout << endl << '\t' << (pToken - 1)->value << endl;
			cout << "Syntax error: missing a $ at the end,the sentence must end with a $" << endl << endl;
		}
		else {
			cout << endl << '\t' << (pToken - 1)->value << pToken->value << endl;
			cout << "Syntax error: expect " << marks[t - id] << " while input is " << marks[token.name - id] << endl << endl;
		}
		cout << "-------------------------------------------------------------------" << endl << endl;
		cout << "Translation stops unexpectedly due to mismatch of symbols" << endl;
		exit(MISMATCH);
	}
}

//判断终结符t是否属于FIRST(B)
bool isFirstB(int t) {
	return (t == id || t == num || t == blank || t == lParenthese || t == intergral || t == sum);
}

//判断终结符t是否属于FOLLOW(F)
bool isFollowF(int t) {
	return (t == rParenthese || t == rBrace || t == dollar);
}

//向html文件输出一个符号
void printToken(int left, int top, int fontSize, bool fontStyle, string token) {
	fprintf(fHtml, "\t\t<div style=\"position: absolute; left:%dpx; top:%dpx;\">\n", left, top);
	fprintf(fHtml, "\t\t\t<span style=\"font-family:%s; font-size:%dpx; font-style:%s; line-height:100%%;\">\n", FONT_FAMILY, fontSize, fontStyle ? "oblique" : "normal");
	fprintf(fHtml, "\t\t\t\t%s\n\t\t\t</span>\n\t\t</div>\n", token.c_str());
}