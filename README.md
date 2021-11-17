# Práctica de Laboratorio #6. Desarrollo dirigido por pruebas
## Introdución
Siguiendo la metodología de desarrollo dirigido por pruebas (Test Driven Development),
se ha implementado una clase Ruby para representar matrices. Se han de definido como tres
operaciones: suma, producto por un escalar y traspuesta.

Una matriz es una cuadrícula utilizada para almacenar o mostrar datos en un formato estructurado. En informática, se puede usar una matriz para almacenar un grupo de datos relacionados. Por ejemplo, algunos lenguajes de programación soportar matrices como tipo de datos que ofrece más flexibilidad que una estática matriz. Al almacenar valores en una matriz en lugar de individualmente las variables, un programa puede acceder y realizar operaciones en los datos de manera más eficiente.

1. El fichero `matrix.rb` contiene una implementación en Ruby de la represntación de matrices. Esta clase recibe un tipo de dato `Array` como entrada.
- `to_s` # devuelve una cadena con la representación de la matriz.
- `Suma` # devuelve una nueva matriz que suma al objeto que invoca el que le pasan como parámetro.
- `Escalar` # devuelve una nueva matriz que multiplica al objeto que invoca por el escalar que le pasan como parámetro.
- `Traspuesta` # devuelve una nueva matriz que surge como resultado de realizar un cambio de columnas por filas y filas por columnas del objeto que invoca.

2. El fichero `matrix_spec.rb` se utiliza para definir los resultados esperados.

## Estructura del árbol de directorios
```bash
.
├── README.md
├── Rakefile
├── lib
│   └── matrix.rb
└── spec
    ├── matrix_spec.rb
    └── spec_helper.rb
```
