# Compilador_Pascal_Simplificado
Trabajo Final del Curso de Compiladores

## Descripción de Implementación
1. Funciones generales:
  - get_char(): Devolver siguiente char y consumirlo.
  - peek_char(): Devolver el siguiente char, pero sin consumirlo. Para saber el siguiente char.
  - scanner(): Verifica el char y devuelve datos del token o error, de acuerdo a las condiciones que cumpla. Utiliza funciones para las condiciones.  
  - gettoken(): Llama al scanner y devuelve un string. Además almacena según sea token o error.
  - get(): Llama a gettoken por cada char, puede cambiar el char si se usa get_char() dentro del gettoken(). Imprime todos los tokens y errores, de acuerdo al orden en que se leen los caracteres.
  
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
  - isWord(): Cuando empieza con una letra, se verifica si es una palabra reservada o un operador (con letras) o un ID. Luego, se les coloca sys tokens.
  - isString(): Cuando empieza por una comilla simple, se verifica el string. Cuando se tienen dos comillas simples, solo se toma una y se almancena para el token. Cuando no tiene fin de comilla simple, se obtiene un error, pero se continúa.
  
  4. Notas:
  - En la función scanner() también se verifican los operadores y delimitadores para obtener un token.
  - En caso no se encuentre su símbolo, se obtiene un error.
  - Todos los errores se almacenan y continúa analizando el scanner.

## Instrucciones para Ejecutar

1. Crear un nuevo proyecto con los archivos main.cpp, scanner.h y programa.txt.
2. El archivo programa.txt puede ser modificado.
3. En el archivo main.cpp colocar la dirección del archivo programa.txt. La dirección que se encuentra dentro de la función get() es una muestra.
4. Ejecutar y se imprimirá en pantalla todos los tokens y errores.
