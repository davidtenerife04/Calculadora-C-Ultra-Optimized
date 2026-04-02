🧮 Calculadora Pro Win32
Potencia nativa en solo 160KB. Una implementación ultraeficiente construida con C++ puro y la API de Windows, diseñada para ser ligera y extremadamente rápida.

🌟 Lo mejor de este proyecto
⚡ Rendimiento de bajo nivel: Escrita directamente sobre la API Win32, sin frameworks pesados, garantizando un consumo mínimo de recursos.

⌨️ Control total por teclado: Incluye soporte completo para el bloque numérico (Numpad) y teclas de función como Esc para borrar o Enter para calcular.

📐 Interfaz Adaptativa: Gracias al evento WM_SIZE, los botones se recalculan dinámicamente para llenar la ventana sin importar su tamaño.

🧠 Gestión de Foco Inteligente: Utiliza técnicas de subclassing para que nunca pierdas la capacidad de escribir con el teclado, incluso después de hacer clic con el ratón.

⌨️ Guía de Atajos
Para que seas más productivo, puedes usar estas teclas directamente:

Números: 0 al 9.

Operadores: +, -, *, /.

Resultado: Enter o la tecla =.

Borrar: Backspace para el último dígito o Esc para limpiar toda la pantalla.

Limpiar Entrada: La tecla Supr activa la función CE.

🚀 Cómo compilar el código
Para obtener el ejecutable (.exe) desde el archivo calculadora.cpp, puedes usar el compilador de Visual Studio (MSVC) con el siguiente comando en la consola:

Bash
cl.exe /O2 /DUNICODE /D_UNICODE calculadora_claude.cpp /link user32.lib gdi32.lib comctl32.lib /SUBSYSTEM:WINDOWS
Nota: Este comando optimiza el tamaño del archivo y habilita el soporte para caracteres Unicode.

🛠️ Detalles técnicos
El núcleo del programa utiliza la fuente Segoe UI para un acabado profesional y maneja la lógica matemática mediante std::wstring y std::wstringstream para asegurar la precisión en los textos de pantalla.

Además, el sistema de subclassing intercepta mensajes como WM_LBUTTONUP para devolver automáticamente el foco a la ventana principal, permitiendo una experiencia de usuario fluida y sin interrupciones.

📜 Licencia
Este es un proyecto de código abierto bajo la licencia MIT. ¡Siéntete libre de usarlo, modificarlo o aprender de él!
