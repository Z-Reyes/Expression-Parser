/*
* Author: Zachary Reyes
* Date: 11/20/17
* This program will automatically tokenize and evaluate a given equation.
* The user will be prompted to give the equation to be evaluated.
* There are several limits to this program.
* 1: powers cannot be raised to non-whole number. If a power is raised to a non-whole power, the decimal portion is truncated.
* 2: Every digit term must be given by a value between 0 and 9.
* 3: The ID can be given only by the values a, b, or c
*/
#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include <math.h>

// This is merely a value that limits the maximum size of the user-given expression
const int maxExpressionSize = 100;
/*
* The Equation struct is a container that holds our main expression value, as well as
* the variable and equal sign of the parse tree
*/
struct Equation {
  struct Expression* main;
  int isEvaluated;
  double value;
  char assignedValue, equals;
};
/*
* The Factor struct is the 'deepest' grammar rule we use. It is utilized for the highest
* priority rules such as exponents and expressions within parentheses.
*/
struct Factor {
  int isEvaluated;
  double value, leftValue, rightValue;
  struct Factor* left;
  struct Factor* right;
  char operator;
  struct Expression* unique;

};
/*
* The Term rule is the second deepest grammar rule, used for rules like multiplication and division.
*/
struct Term {
  int isEvaluated;
  double value, leftValue, rightValue;
  struct Term* left;
  struct Factor* right;
  char operator;
};
/*
* The Expression rule is the shallowest grammar rule. It is used for simple addition
* and subtraction between expressions and terms.
*/
struct Expression {
  int isEvaluated;
  double value, leftValue, rightValue;
  struct Expression* left;
  struct Term* right;
  char operator;
} ;
/*
* These are some function declarations so that the GCC compiler knows they exist.
*/
double getFactorValue(struct Factor*);
void calculateFactor(struct Factor*);
double getTermValue (struct Term*);
void calculateTerm (struct Term*);
double getExpressionValue(struct Expression*);
void calculateExpression(struct Expression*);
struct Expression* EvaluateExpression(char*);
struct Term* EvaluateTerm(char*);
struct Factor* EvaluateFactor(char*);

/*
* The EvaluateEquation function is the function that runs the evaluation of an equation rule
* (defined as <Equation> -> <id> <equal_sign> <Expression>).
* Input: inp is the char array that represents the entire string of the equation.
* Output: If the char array is a valid Equation, a parse tree of that Equation will be returned,
* with the returned Equation struct being the root of the parse tree.
*/
struct Equation* EvaluateEquation (char* inp) {
  struct Equation* final = calloc(1,sizeof(struct Equation));
  int m = strlen(inp);
  if (m < 3) {
    fprintf(stderr, "%s\n", "Syntax is not satisfied. Need an expression on right side of equals sign." );
    exit(EXIT_FAILURE);
  }
  if (*inp == 'a' || *inp == 'b' || *inp == 'c') {
    final->assignedValue = *(inp);
  } else {
    fprintf(stderr, "%s\n", "Syntax is not satisfied. Need a valid identifier on left side of equals sign" );
    exit(EXIT_FAILURE);
  }
  if (*(inp + 1) == '=') {
    final->equals = *(inp + 1);
  } else {
    fprintf(stderr, "%s\n", "Syntax is not satisfied. Need a equals sign between identifier and expression" );
    exit(EXIT_FAILURE);
  }
  char* rightSide = calloc(m-1, sizeof(char));
  memcpy(rightSide, (inp + 2), m-2);
  //printf("%s%s\n", "Here is expression: ", rightSide );
  final->main = EvaluateExpression(rightSide);
  //free(rightSide);

  return final;
}
/*
* The getEquationValue function returns the value of a valid Equation struct.
* It operates in a semi-recursive fashion, getting its value by calling the getExpressionValue method
* on its Expression pointer.
* Input: The root of the parse tree for a valid Equation (Equation* in) is given
* Output: in's value variable will have the correct value for the equation.
*/
double getEquationValue (struct Equation* in) {
    if (in->isEvaluated == 0) {
      in->value = getExpressionValue(in->main);
      in->isEvaluated = 1;
      return in->value;
    } else {
      return in->value;

    }
}
/*
* The findFirstGivenCharacters function reads a given string right to left in search for given
* lexemes.
* Input: val1 is the first lexeme to be searched for. val2 is the second lexeme to to be searched for.
* evaluatedString is the string to be searched in in an attempt to locate the position of either val1 or val2.
* Output: An integer specifying the position of a found lexeme is returned. If
* the given lexemes are not found in the string, we return -1.
*/
int findFirstGivenCharacters(char val1, char val2, char* evaluatedString) {
  int found = 0;
  int bounded = 0;
  int index = strlen(evaluatedString) - 1;
  char currentVal;
  //Find the correct characters
  while (!found && index >= 0) {
    currentVal = *(evaluatedString + index);
    //If the index value is equal to one of the characters we're looking for,
    //we can exit because now we know the correct location.
    if (currentVal == ')') {
      bounded++;
    } else if (currentVal == '(') {
      bounded--;
    }
    if ((currentVal == val1 || currentVal == val2) && !bounded) {
      found = 1;
    } else {
      index--;
    }
  }
  if (bounded != 0) {
    fprintf(stderr, "Parentheses are not balanced!\n");
    exit(EXIT_FAILURE);
  }

return index;

}
/*
* The getFactorValue function returns the value of a valid Factor struct.
* It operates in a semi-recursive fashion, getting its value by calling the calculateFactor function
* on itself. If the input has already been evaluated, we simply return its value.
* Input: The root of the parse tree for a valid Factor (Factor* input) is given
* Output: in's value variable will have the correct value for the factor.
*/
double getFactorValue (struct Factor* input) {
  if (input->isEvaluated) {
    return input->value;
  } else {
    calculateFactor(input);
    return input->value;
  }
}
/*
* The calculateFactor function calculates the value of a Factor struct by using
* recursion to dig for values deeper inside the tree.
* Input: The Factor* in is a valid Factor rule.
* Output: The Factor* in will have the correct value for its Factor rule.
*/
void calculateFactor(struct Factor* in) {
  if (in->unique != NULL && in->left == NULL && in->right == NULL) {
    in->value = getExpressionValue(in->unique);
  } else if (in->unique == NULL && in->left != NULL && in->right != NULL && in->operator == '^') {
    in->leftValue = getFactorValue(in->left);
    in->rightValue = getFactorValue(in->right);
  //  printf("%f\n", in->leftValue );
  //  printf("%f\n", in->rightValue);
    double tempValue = 1.0;
    int num = (int) in->rightValue;
  //  printf("%d\n", num);
    int mag = 0;
    if (num < 0) {
      mag = num * -1;
    } else {
      mag = num;
    }
    for (int i = 0; i < mag; i++) {
      tempValue = tempValue * in->leftValue;
    }
    if (num < 0) {
      in->value = 1/tempValue;
    } else {
    in->value = tempValue;
  }
    in->isEvaluated = 1;
  } else {
    //an error has occurred.
  }
}
/*
* The getTermValue function returns the value of a valid Term struct.
* It operates in a semi-recursive fashion, getting its value by calling the calculateTerm function
* on itself. If the input has already been evaluated, we simply return its value.
* Input: The root of the parse tree for a valid Term (Term* input) is given
* Output: in's value variable will have the correct value for the term.
*/
double getTermValue (struct Term* input) {
  if (input->isEvaluated) {
    return input->value;
  } else {
    calculateTerm(input);
    return input->value;
  }
}
/*
* The calculateTerm function calculates the value of a Term struct by using
* recursion to dig for values deeper inside the tree.
* Input: The Term* in is a valid Term rule.
* Output: The Term* in will have the correct value for its Term rule.
*/
void calculateTerm (struct Term* in) {
  if (in->right != NULL && in->left == NULL) {
    //Value will equal right term value.
    //Make sure to call calculateExpresion(termVersion) first.
    in->value = getFactorValue(in->right);
    in->isEvaluated = 1;
  } else if (in->right != NULL && in->left != NULL) {

    //caculateTerm(in->right);
    in->leftValue = getTermValue(in->left);
    in->rightValue = getFactorValue(in->right);
    //change the values.

    //Then set value equal to the sum or difference of values.
    if (in->operator == '*') {
      in->value = in->leftValue * in->rightValue;
    } else if (in->operator == '/') {
      in->value = in->leftValue / in->rightValue;
    }
    in->isEvaluated = 1;
  } else {
    //Do nothing. An error has occurred?
  }
}
/*
* The getExpressionValue function returns the value of a valid Expression struct.
* It operates in a semi-recursive fashion, getting its value by calling the calculateExpression function
* on itself. If the input has already been evaluated, we simply return its value.
* Input: The root of the parse tree for a valid Expression (Expression* input) is given
* Output: in's value variable will have the correct value for the expression.
*/
double getExpressionValue(struct Expression* input) {

  if (input->isEvaluated) {
    return input->value;
  } else {
    calculateExpression(input);
    return input->value;
  }
}
/*
* The calculateExpression function calculates the value of an Expression struct by using
* recursion to dig for values deeper inside the tree.
* Input: The Expression* in is a valid Expression rule.
* Output: The Expression* in will have the correct value for its Expression rule.
*/
void calculateExpression(struct Expression* in) {
  //First, see if only right term is set
  if (in->right != NULL && in->left == NULL) {
    //Value will equal right term value.
    //Make sure to call calculateExpresion(termVersion) first.
    in->value = getTermValue(in->right);
    in->isEvaluated = 1;
  } else if (in->right != NULL && in->left != NULL) {

    //caculateTerm(in->right);
    in->leftValue = getExpressionValue(in->left);
    in->rightValue = getTermValue(in->right);
    //change the values.

    //Then set value equal to the sum or difference of values.
    if (in->operator == '+') {
      in->value = in->leftValue + in->rightValue;
    } else if (in->operator == '-') {
      in->value = in->leftValue - in->rightValue;
    }
    in->isEvaluated = 1;
  } else {
    //Do nothing.
  }
}
/*
* The EvaluateExpression function takes a string and determines whether or not it
* is a valid string for the expression grammar rule.
* Input: item is a char* that is going to be checked if it is a valid expression.
* If so, the function will use it to create an Expression node in the parse tree.
* Output: If item is a valid string, an Expression* is returned. If not, the program is
* halted.
*/
struct Expression* EvaluateExpression(char* item) {
  int stringSize = strlen(item);
  if (stringSize == 0) {
    fprintf(stderr, "There are too many operators!\n");
    exit(EXIT_FAILURE);
  }
  struct Expression* tier1 = calloc(1,sizeof(struct Expression));

  //
  //Find the implementation here
  //
  int foundIndex = findFirstGivenCharacters('+','-',item);
  if (foundIndex == -1) {
      //If Expression not found, just leave it and call EvaluateTerm

      tier1->right = EvaluateTerm(item);

  } else {


    //This else will play if an expression is found.
    //Set the current Expression's operator
    tier1->operator = *(item + foundIndex);

    char* left = calloc(stringSize, sizeof(char));
    char* right = calloc(stringSize,sizeof(char));

    memcpy(left, item, foundIndex);
    //Could be a breaking point.
    char* adjusted = item + foundIndex + 1;
    memcpy(right, adjusted, stringSize-foundIndex);
    tier1->left = EvaluateExpression(left);
    tier1->right = EvaluateTerm(right);
    free(item);

  }

  return tier1;
}
/*
* The EvaluateTerm function takes a string and determines whether or not it
* is a valid string for the term grammar rule.
* Input: item is a char* that is going to be checked if it is a valid term.
* If so, the function will use it to create a term node in the parse tree.
* Output: If item is a valid string, a Term* is returned. If not, the program is
* halted.
*/
struct Term* EvaluateTerm(char* item) {
  int stringSize = strlen(item);
  if (stringSize == 0) {
    fprintf(stderr, "There are too many operators!\n");
    exit(EXIT_FAILURE);
  }
  struct Term* tier2 = calloc(1, sizeof(struct Term));
  int foundIndex = findFirstGivenCharacters('*','/',item);
  if (foundIndex == -1) {
    tier2->right = EvaluateFactor(item);
  } else {
    tier2->operator = *(item + foundIndex);

    char* left = calloc(stringSize, sizeof(char));
    char* right = calloc(stringSize, sizeof(char));

    memcpy(left, item, foundIndex);
    //Could be a breaking point.
    char* adjusted = item + foundIndex + 1;
    memcpy(right, adjusted, stringSize-foundIndex);
    tier2->left = EvaluateTerm(left);
    tier2->right = EvaluateFactor(right);
    free(item);
  }
  //

  return tier2;
}
/*
* The EvaluateFactor function takes a string and determines whether or not it
* is a valid string for the Factor grammar rule.
* Input: item is a char* that is going to be checked if it is a valid Factor.
* If so, the function will use it to create a Factor node in the parse tree.
* Output: If item is a valid string, an Factor* is returned. If not, the program is
* halted.
*/
struct Factor* EvaluateFactor(char* item) {
  struct Factor* tier3 = calloc(1, sizeof(struct Factor));
  char j = *item;
  int testSingleChar = strlen(item);
  char end = *(item + testSingleChar-1);
  //Now we gotta do something different. First we need to check and make sure
  //that we don't have just a single digit.
  if (testSingleChar == 0) {
    fprintf(stderr, "There are too many operators!\n");
    exit(EXIT_FAILURE);
  }
  if (testSingleChar == 1) {
    if (j >= 48 && j <= 57) {
      tier3->value = j - 48;
      tier3->isEvaluated = 1;
    } else {
      fprintf(stderr, "An invalid character was used!!!\n");
      exit(EXIT_FAILURE);
    }
  } else if (j == '(' && end == ')') {
    char* subExpression = calloc(testSingleChar, sizeof(char));
    memcpy(subExpression, (item + 1), (testSingleChar - 2));
    tier3->unique = EvaluateExpression(subExpression);
  } else {
    //Now we check for factors.
    int foundIndex = findFirstGivenCharacters('^','^', item);
    if (foundIndex == -1) {
      fprintf(stderr, "An invalid character was used!!!\n");
      exit(EXIT_FAILURE);
    }
    tier3->operator = *(item + foundIndex);
    char* left = calloc(testSingleChar, sizeof(char));
    char* right = calloc(testSingleChar,sizeof(char));
    //int stringSize = strlen(item);
    memcpy(left, item, foundIndex);
    //Could be a breaking point.
    char* adjusted = item + foundIndex + 1;
    memcpy(right, adjusted, testSingleChar-foundIndex);
    tier3->left = EvaluateFactor(left);
    tier3->right = EvaluateFactor(right);

  }

//free(item);
  return tier3;
}
/*
* The filterWhiteSpace function simply removes the newline and whitespace characters
* in a given string.
* Input: char* t is a string that may or may not have whitespaces and newline characters.
* Output: A char* with no whitespaces or newline characters is returned.
*/
char* filterWhiteSpace(char* t) {
  int m = strlen(t);
  char* re = calloc(m, sizeof(char));
  int counter = 0;
  //printf("%d\n", m);
  for (int i = 0; i < m; i++) {
    char n = *(t + i);
    if (n != 32 && n != 10) {
      *(re + counter) = *(t + i);
      counter++;
    }
  }
  //*(re+counter) = 0;
  return re;
}
/*
* The tokenizer function takes a given string and tokenizes each of its lexemes.
* If a lexeme is found that is incompatible with our grammar, we halt the program.
* Input: char* in is the string to be tokenized.
* Output: A char* with all of the lexemes labeled is returned.
*/
char* tokenizer(char* in) {
  char maxSize[(10 * maxExpressionSize) + 3];
  char* id = "id,";
  char* leftP = "lt_paren,";
  char* rightP = "rt_paren,";
  char* plus = "add_op,";
  char* minus = "sub_op,";
  char* multi = "mul_op,";
  char* division = "div_op,";
  char* power = "pow_op,";
  char* equals = "equal_sign,";

  int currIndex = 0;
    *(maxSize + currIndex) = '[';
  int count = 0;
  char* tempString;
  currIndex++;
  int j = 0;
  char* digitHolder = calloc(3, sizeof(char));
  while (*(in + j) != 0) {
    if (*(in + j) == 'a' || *(in + j) == 'b' || *(in + j) == 'c') {
      tempString = id;
    } else if (*(in + j) == '(') {
      tempString = leftP;
    } else if (*(in + j) == ')') {
      tempString = rightP;
    } else if (*(in + j) == '+') {
      tempString = plus;
    } else if (*(in + j) == '-') {
      tempString = minus;
    } else if (*(in + j) == '*') {
      tempString = multi;
    } else if (*(in + j) == '/') {
      tempString = division;
    } else if (*(in + j) == '^') {
      tempString = power;
    } else if (*(in + j) == '=') {
      tempString = equals;
    } else if (*(in + j) >= 48 && *(in + j) <= 57) {
      //char temporary = *(in + j);
      memcpy(digitHolder, (in + j), 1);
      *(digitHolder + 1) = ',';
      tempString = digitHolder;
    } else {
      fprintf(stderr, "%s\n", "Bad input! Try again using only valid lexemes!");
      exit(EXIT_FAILURE);
    }
    while (*(tempString + count) != 0) {
      *(maxSize + currIndex) = *(tempString + count);
      currIndex++;
      count++;
    }
    count = 0;
    j++;
  }
  *(maxSize + currIndex-1) = ']';
  *(maxSize + currIndex) = 0;
  int sizes = strlen(maxSize);
  char* returnedString = calloc((sizes + 1), sizeof(char));
  strcpy(returnedString, maxSize);
  free(digitHolder);
  return returnedString;

}
/*
* Our main function merely initiates the program. It prompts the user for a valid equation,
* and then returns the result (assuming the equation was valid).
*/
int main() {

   //int j = factorial(5);
   printf("%s\n", "Please input a valid equation (example: a = 3 + 4 ):");
   char* jk = calloc(1,sizeof(char));
   fgets(jk, maxExpressionSize, stdin);
   char* k = filterWhiteSpace(jk);
   char* tokenizedString = tokenizer(k);
   printf("%s%s\n", "Here is the tokenized String: ", tokenizedString );

   struct Equation* testxx = EvaluateEquation(k);
   printf("%c %c %f\n", testxx->assignedValue, testxx->equals, getEquationValue(testxx));
  
  // free(receive);
  // free(test);

   //printf("%s\n", test);
   return 0;
}
