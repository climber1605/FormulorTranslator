#include<iostream>
#include<cmath>
#include<string>
using namespace std;

#define max(a,b) ((a)>(b) ? (a):(b))
#define MAX 200		//�ʷ��Ǻ������ֵ
#define NONE 0x300   //��ʼ�ʷ��Ǻ���
#define INIT_FONTSIZE 80	//��ʼfontsize����ֵ����Ĭ�������С
#define INIT_LEFT 100	//��ʼleft����ֵ
#define INIT_TOP 100	//��ʼtop����ֵ
#define FONT_FAMILY "Monaco"	//��������
const double leftOffset = 0.6;	//���ڼ����±��left����ֵ�ĳ���
const double topOffset = 0.6;	//���ڼ����±��top����ֵ�ĳ���
const double fontsizeShrink = 0.5;	//���ڼ������±��font-size����ֵ�ĳ���

									//�������״̬
#define SUCCEED 0
#define SPELL_ERROR -1
#define UNDEFINED_SYMBOL -2
#define INCOMPLETE -3
#define MISMATCH -4
#define UNDEFINED_ITEM -5
#define OPEN_FILE_FAIL -6
#define UNKNOWN_MISTAKE  -7

									//����ʷ��Ǻ���
const int id = 0x100;		//��ʶ��
const int num = 0x101;		//�޷�������
const int blank = 0x102;	//�ո��
const int intergral = 0x103;	//���ֺ�
const int sum = 0x104;		//��ͺ�
const int fullScript = 0x105;	//���±�
const int superScript = 0x106;	//�ϱ�
const int subScript = 0x107;	//�±�
const int lParenthese = 0x108;	//��Բ����
const int rParenthese = 0x109;	//��Բ����
const int lBrace = 0x10A;	//������
const int rBrace = 0x10B;	//�һ�����
const int dollar = 0x10C;	//$��

							//�����͵ļǺ���ӳ����ַ���
string marks[] = { "id","num","blank","\\int","\\sum","_^","^","_","(",")","{","}","$" };

struct tokenType {
	int name;	//�Ǻ���
	string value;	//����ֵ
}tokens[MAX];		//tokens���ڴ�Ŵʷ�������ɨ���ļ���õ��ļǺ���

tokenType *pToken = tokens;		//���봮ָ��
tokenType token;	//��ǰ���봮ָ����ָ��ļǺ�
FILE *fHtml;	//����ļ�ָ��

int scanner(FILE *fin);		//�ʷ���������ɨ�������ļ������üǺ��������tokens������
int fail(int start);		//����ǰ״̬��Ϊ��һ��״̬ת��ͼ�Ŀ�ʼ״̬��׼��ƥ����һ״̬ת��ͼ
void parser(void);			//�﷨������������ʹ�õݹ��½���Ԥ������������߷����߽���
void S(int left, int top, int fontSize);
void B(int &left, int top, int fontSize, bool fontStyle);
void F(int &left, int top, int fontSize);
void match(int t);			//�жϵ�ǰ�Ǻ��Ƿ��������ļǺ�tƥ��
bool isFirstB(int t);		//�ж��ս��t�Ƿ�����FIRST(B)
bool isFollowF(int t);		//�ж��ս��t�Ƿ�����FOLLOW(F)
							//��html�ļ����һ������
void printToken(int left, int top, int fontsize, bool fontstyle, string tokenVal);
int main(int argc, char * argv[]) {
	FILE *fin, *fout;
	char ch;
	if (argc == 1) {
		cout << "��������ӣ���crtl+Z������" << endl;
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

//�ʷ���������ɨ�������ļ������üǺ��������tokens������
int scanner(FILE *fin) {
	int state = 0;	//DFA��ʼ״̬Ϊ0
	int i = 0;	//�ʷ���Ԫָ��
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
			tokens[i].name = id;	//ʶ����Ǻ�id
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//׼��ʶ����һ�Ǻ�
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
			tokens[i].name = num;	//ʶ����Ǻ�num
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//׼��ʶ����һ�Ǻ�
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
			tokens[i].name = blank;	//ʶ����Ǻ�\blank
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//׼��ʶ����һ�Ǻ�
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
			tokens[i].name = intergral;	//ʶ����Ǻ�\int
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//׼��ʶ����һ�Ǻ�
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
			tokens[i].name = sum;	//ʶ����Ǻ�\sum
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//׼��ʶ����һ�Ǻ�
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
			tokens[i].name = fullScript;	//ʶ����Ǻ�_^
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//׼��ʶ����һ�Ǻ�
			break;
		case 22:
			fseek(fin, -1, SEEK_CUR);
			tokens[i].name = subScript;	//ʶ����Ǻ�_
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//׼��ʶ����һ�Ǻ�
			break;
		case 23:
			tokens[i].name = superScript;	//ʶ����Ǻ�^
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//׼��ʶ����һ�Ǻ�
			break;
		case 24:
			tokens[i].name = lParenthese;	//ʶ����Ǻ�(
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//׼��ʶ����һ�Ǻ�
			break;
		case 25:
			tokens[i].name = rParenthese;	//ʶ����Ǻ�)
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//׼��ʶ����һ�Ǻ�
			break;
		case 26:
			tokens[i].name = lBrace;	//ʶ����Ǻ�{
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//׼��ʶ����һ�Ǻ�
			break;
		case 27:
			tokens[i].name = rBrace;	//ʶ����Ǻ�}
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//׼��ʶ����һ�Ǻ�
			break;
		case 28:
			tokens[i].name = dollar;	//ʶ����Ǻ�$
			cout << "< " << marks[tokens[i].name - id] << ',' << tokens[i].value << " >" << endl;
			i++;
			state = 0;	//׼��ʶ����һ�Ǻ�
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

//����ǰ״̬��Ϊ��һ��״̬ת��ͼ�Ŀ�ʼ״̬��׼��ƥ����һ��״̬ת��ͼ
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

//�﷨������������ʹ�õݹ��½���Ԥ������������߷����߼��㣬����Ǻ��Ѿ�ȫ�������tokens������
void parser(void) {
	token = tokens[0];
	while (token.name != NONE) {
		int left = INIT_LEFT, top = INIT_TOP, fontSize = INIT_FONTSIZE;
		S(left, top, fontSize);
	}
}

void S(int left, int top, int fontSize) {
	if (token.name == dollar) {		//����ʽS-->$BF$
		cout << "S-->$BF$" << endl;
		match(dollar);
		B(left, top, fontSize, false);
		F(left, top, fontSize);
		match(dollar);
	}
	else {	//�﷨����
		cout << endl << '\t' << token.value << endl;
		cout << "Syntax error: missing a $ at the beginning,the sentence must begin with a $" << endl << endl;
		cout << "-------------------------------------------------------------------" << endl << endl;
		cout << "Translation stops unexpectedly due to unknown grammar production" << endl;
		exit(UNDEFINED_ITEM);
	}
}

void B(int &left, int top, int fontSize, bool fontStyle) {
	if (token.name == id) {		//����ʽB-->id
		cout << "B-->id" << endl;
		fontStyle = true;
		printToken(left, top, fontSize, fontStyle, token.value);
		left += static_cast<int>(floor(fontSize*token.value.length() * leftOffset));
		match(id);
	}
	else if (token.name == num) {	//����ʽB-->num
		cout << "B-->num" << endl;
		printToken(left, top, fontSize, fontStyle, token.value);
		left += static_cast<int>(floor(fontSize*token.value.length() * leftOffset));
		match(num);
	}
	else if (token.name == blank) {		//����ʽB-->\blank
		cout << "B-->\\blank" << endl;
		printToken(left, top, fontSize, fontStyle, " ");
		left += static_cast<int>(floor(fontSize * leftOffset));
		match(blank);
	}
	else if (token.name == lParenthese) {	//����ʽB-->(BF)
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
	else if (token.name == intergral) {		//����ʽB-->\int{BF}{BF}{BF}
		cout << "B-->\\int{BF}{BF}{BF}" << endl;
		int newFontSize = static_cast<int>(floor(fontSize * 1.1));
		printToken(left, top, newFontSize, fontStyle, "&int;");
		left += static_cast<int>(floor(newFontSize * 0.3));
		match(intergral);
		//ƥ���±�
		match(lBrace);
		int curLeft = left;	//��ʱ����left���ԣ���Ϊ��������ϱ�ʱ��left����
		int newTop = top + static_cast<int>(floor(fontSize * 0.7));
		newFontSize = static_cast<int>(floor(fontSize * fontsizeShrink));
		B(left, newTop, newFontSize, fontStyle);

		F(left, newTop, newFontSize);
		match(rBrace);
		//ƥ���ϱ�
		match(lBrace);
		newTop = top - static_cast<int>(floor(fontSize * 0.3));
		newFontSize = static_cast<int>(floor(fontSize * fontsizeShrink));
		B(curLeft, newTop, newFontSize, fontStyle);
		F(curLeft, newTop, newFontSize);
		match(rBrace);
		//ƥ���������
		match(lBrace);
		left = max(left, curLeft);
		B(left, top, fontSize, fontStyle);
		F(left, top, fontSize);
		match(rBrace);
	}
	else if (token.name == sum) {		//����ʽB-->\sum{BF}{BF}{BF}
		cout << "B-->\\sum{BF}{BF}{BF}" << endl;
		int newFontSize = static_cast<int>(floor(fontSize * 1.1));
		printToken(left, top, newFontSize, fontStyle, "&Sigma;");
		left += static_cast<int>(floor(fontSize * 0.75));
		match(sum);
		//ƥ���±�
		match(lBrace);
		int curLeft = left;	//��ʱ����left���ԣ���Ϊ��������ϱ�ʱ��left����
		int newTop = top + static_cast<int>(floor(fontSize * topOffset));
		newFontSize = static_cast<int>(floor(fontSize * fontsizeShrink));
		B(left, newTop, newFontSize, fontStyle);
		F(left, newTop, newFontSize);
		match(rBrace);
		//ƥ���ϱ�
		match(lBrace);
		B(curLeft, top, newFontSize, fontStyle);
		F(curLeft, top, newFontSize);

		match(rBrace);
		//ƥ���������
		match(lBrace);
		left = max(left, curLeft);
		B(left, top, fontSize, fontStyle);
		F(left, top, fontSize);
		match(rBrace);
	}
	else {		//�﷨����
		cout << endl << '\t' << (pToken - 1)->value << pToken->value << endl;
		cout << endl << "Syntax error: expect id,num,\\blank,(,\\int or \\sum while input is " << marks[token.name - id] << endl << endl;
		cout << "-------------------------------------------------------------------" << endl << endl;
		cout << "Translation stops unexpectedly due to unknown grammar production" << endl;
		exit(UNDEFINED_ITEM);
	}
}

void F(int &left, int top, int fontSize) {
	if (token.name == fullScript) {		//����ʽF-->_^{BF}{BF}F
		cout << "F-->_^{BF}{BF}F" << endl;
		match(fullScript);
		//ƥ���±�
		match(lBrace);
		int curLeft = left;	//��ʱ����left���ԣ���Ϊ��������ϱ�ʱ��left����
		int newTop = top + static_cast<int>(floor(fontSize * topOffset));
		int newFontSize = static_cast<int>(floor(fontSize * fontsizeShrink));
		B(left, newTop, newFontSize, false);
		F(left, newTop, newFontSize);
		match(rBrace);
		//ƥ���ϱ�
		match(lBrace);
		B(curLeft, top, newFontSize, false);
		F(curLeft, top, newFontSize);
		match(rBrace);
		//ƥ��ʣ�ಿ��
		left = max(left, curLeft);
		F(left, top, fontSize);
	}
	else if (token.name == superScript) {	//����ʽF-->^{BF}F
		cout << "F-->^{BF}F" << endl;
		match(superScript);
		//ƥ���ϱ�
		match(lBrace);
		int newFontSize = static_cast<int>(floor(fontSize * fontsizeShrink));
		B(left, top, newFontSize, false);
		F(left, top, newFontSize);
		match(rBrace);
		//ƥ��ʣ�ಿ��
		F(left, top, fontSize);
	}
	else if (token.name == subScript) {		//����ʽF-->_{BF}F
		cout << "F-->_{BF}F" << endl;
		match(subScript);
		//ƥ���±�
		match(lBrace);
		int newTop = top + static_cast<int>(floor(fontSize * topOffset));
		int newFontSize = static_cast<int>(floor(fontSize * fontsizeShrink));
		B(left, newTop, newFontSize, false);
		F(left, newTop, newFontSize);
		match(rBrace);
		//ƥ��ʣ�ಿ��
		F(left, top, fontSize);
	}
	else if (isFirstB(token.name)) {		//����ʽF-->BF
		cout << "F-->BF" << endl;
		B(left, top, fontSize,false);
		F(left, top, fontSize);
	}
	else if (isFollowF(token.name) || token.name == NONE) {		//����ʽF-->NULL
		cout << "F--> " << endl;
	}
	else {	//�﷨����
		cout << endl << '\t' << (pToken - 1)->value << pToken->value << endl;
		cout << endl << "Syntax error: expect id,num,\\blank,(,),},_^,_,^,\\int,\\sum or $ while input is " << marks[token.name - id] << endl << endl;
		cout << "-------------------------------------------------------------------" << endl << endl;
		cout << "Translation stops unexpectedly due to unknown grammar production" << endl;
		exit(UNDEFINED_ITEM);
	}
}

//�жϵ�ǰ�Ǻ��Ƿ��������ļǺ�tƥ��
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

//�ж��ս��t�Ƿ�����FIRST(B)
bool isFirstB(int t) {
	return (t == id || t == num || t == blank || t == lParenthese || t == intergral || t == sum);
}

//�ж��ս��t�Ƿ�����FOLLOW(F)
bool isFollowF(int t) {
	return (t == rParenthese || t == rBrace || t == dollar);
}

//��html�ļ����һ������
void printToken(int left, int top, int fontSize, bool fontStyle, string token) {
	fprintf(fHtml, "\t\t<div style=\"position: absolute; left:%dpx; top:%dpx;\">\n", left, top);
	fprintf(fHtml, "\t\t\t<span style=\"font-family:%s; font-size:%dpx; font-style:%s; line-height:100%%;\">\n", FONT_FAMILY, fontSize, fontStyle ? "oblique" : "normal");
	fprintf(fHtml, "\t\t\t\t%s\n\t\t\t</span>\n\t\t</div>\n", token.c_str());
}