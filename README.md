# Compilador_Pascal_Simplificado
Trabajo Final del Curso de Compiladores

## Descripción de Implementación SCANNER
1. Funciones generales:
  - get_char(): Devolver siguiente char y consumirlo.
  - peek_char(): Devolver el siguiente char, pero sin consumirlo. 
  - scanner(): Verifica el char y devuelve datos del token o error, de acuerdo a las condiciones que cumpla. Utiliza funciones para las condiciones.  
  - gettoken(): Llama al scanner y devuelve un string. Además, almacena según sea token o error.
  - get(): Llama a gettoken() por cada char, puede cambiar el char si se usa get_char() dentro del gettoken(). Imprime todos los tokens y errores, de acuerdo al orden en que se leen los caracteres.
  
2. Vectores:
  - Tokens: Almacena los tokens como strings.
  - Errors: Almacena los errores como strings.
  - Operators: ALmacena los operadores como un pair<string, string>, el primer string es para el tag y el segundo es para el valor.
  - Delimiters: ALmacena los delimitadores como un pair<string, string>, el primer string es para el tag y el segundo es para el valor.
  - Reserved_words: ALmacena las palabras reservadas como un pair<string, string>, el primer string es para el tag y el segundo es para el valor.

3. Funciones utilizadas en Scanner():
  - skip_comment(): Cuando se encuentra un comentario no se consume, es decir, se salta y no tiene un token. Los comentarios se representan con {} o (* *).
  - skip_space(): No consume/salta cuando encuentra un espacio, tabulación o final de línea.
  - isNumber(): Si empieza con un número, verifica si es un número o un error. También tiene el caso cuando es un exponente decimal con un signo (si no tiene signo se toma como +). Todos los números se convierten a doubles y se les coloca su token.
  - isWord(): Cuando empieza con una letra, se verifica si es una palabra reservada o un operador (con letras) o un ID. Luego, se les coloca sus tokens.
  - isString(): Cuando empieza por una comilla simple, se verifica el string. Cuando se tienen dos comillas simples, solo se toma una y se almancena para el token. Cuando no tiene fin de comilla simple, se obtiene un error, pero se continúa.
  
  4. Notas:
  - En la función scanner() también se verifican los operadores y delimitadores para obtener un token.
  - En caso no se encuentre un símbolo, se obtiene un error.
  - Todos los errores se almacenan y continúa analizando el scanner.

## Descripción de Implementación PARSER
1. Funciones generales:
  - get_vector(): Toma todos los tokens del scanner y los guarda en el vector tags.
  - next_token(): Toma el siguiente token del vector tags con su tag y su valor.
  - print_error(): Imprimir el error encontrado.
  - add_node(): Insert un nuevo nodo en el árbol, indicando el padre y el nombre del hijo.
  - parse_tree(): Para imprimir recursivamente el árbol, con un | y con espacios para mostrar donde se encuentra el padre.
  - parser(): Función que llama a la clase parser y verifica según la gramática. Si tiene un error, lo imprime y termina el proceso.
             Si no tiene error, imprime el árbol de parseo.
             
2. Datos generales:
  - Se tiene una estructura para el nodo del árbol, para el árbol y para el parser.
  - La estructura parser tiene todas las funciones según la gramática generada (vea grammar.txt).
  - El árbol de parseo continúa hasta que no haya más no-terminales.
  - Incluye la Recuperación de errores: Consume y llama a la función next_token(), mientras no sea parte de los Siguientes(FOLLOW'S). De esa manera pasa a la siguiente producción.

## Instrucciones para Ejecutar

1. Crear un nuevo proyecto con los archivos main.cpp, scanner.h, parser.h y programa.txt.
2. El archivo programa.txt puede ser modificado (archivo del cual va a leer el scanner).
3. En el archivo main.cpp colocar la dirección donde se encuentra el archivo programa.txt. La dirección dentro de la función get() es una muestra, no funciona.
4. Ejecutar y se imprimirá en pantalla:
  - Todos los tokens y errores del scanner. 
  - Todos los tokens que recibe el parser y verifica. 
  - Cuando termine el parser imprimirá la lista de errores en caso la lista no está vacía.
  - Si la lista está vacía, imprime el árbol de parseo.

##### NOTA: En los archivos example_output.txt y example_error_output.txt, se encuentran los ejemplos de lo que se imprime después de ejecutar según las indicaciones. EL primer .txt es el ejemplo del caso cuando el programa está correcto e imprime el árbol de parseo. Mientras que, el segundo .txt es el ejemplo para el caso cuando la lista de errores no está vacía. Además, se utilizaron los ejemplos que están en programa.txt para el proceso. Probar cada programa por separado.
