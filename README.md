# 📊 Display 2.8" com ESP32 — Painel do Instagram

Display

https://meli.la/2koUS81

Antena

https://meli.la/1MLQvft


<img width="4032" height="3024" alt="display" src="https://github.com/user-attachments/assets/0b8f3c5e-eb5f-4ba5-9834-7ffd958ddcca" />


Projeto desenvolvido para transformar um **ESP32 com display TFT Touch de 2.8"** em um pequeno painel inteligente capaz de consultar APIs e exibir informações do Instagram em tempo real.

O objetivo do projeto é demonstrar, de forma prática, como dispositivos IoT podem consumir dados de serviços externos através de **APIs REST**, processar respostas em **JSON** e apresentar essas informações em uma interface gráfica.

## 🚀 Funcionalidades

* 📶 Conexão do ESP32 à internet via Wi-Fi
* 📡 Consulta de APIs através de requisições HTTP
* 📊 Exibição da quantidade de seguidores do Instagram
* 💬 Exibição de comentários
* 🔄 Atualização automática das informações
* 🖥️ Interface gráfica em display TFT 2.8"
* 👆 Navegação através do Touch
* 🧠 Processamento das respostas JSON diretamente pelo ESP32
* 📱 Integração com dados do Instagram

## 🛠️ Tecnologias utilizadas

### ESP32

O ESP32 é o microcontrolador responsável por conectar o projeto à internet, consultar as APIs, processar os dados recebidos e controlar a interface do display.

### Display TFT Touch 2.8"

Responsável pela interface visual do projeto, permitindo apresentar informações de maneira muito mais amigável do que utilizando apenas o Monitor Serial.

### Wi-Fi

O ESP32 utiliza a conexão Wi-Fi para acessar os servidores e realizar as consultas necessárias.

### HTTP / HTTPS

As informações são obtidas através de requisições realizadas pela internet.

### JSON

Grande parte das APIs retorna os dados no formato JSON.

O ESP32 recebe essa resposta e extrai somente as informações necessárias para apresentar no display.

## 🌐 O que é uma API?

**API — Application Programming Interface** ou **Interface de Programação de Aplicações** é uma forma padronizada de permitir que diferentes sistemas se comuniquem.

Neste projeto podemos imaginar o seguinte fluxo:

```text
Instagram / Serviço
        ↓
       API
        ↓
     Internet
        ↓
      ESP32
        ↓
 Processamento JSON
        ↓
 Display TFT 2.8"
```

Ou seja, o ESP32 não precisa conhecer como todo o sistema do Instagram funciona internamente.

Ele faz uma solicitação para uma API, recebe os dados e utiliza apenas as informações necessárias.

## 📡 APIs utilizadas

O projeto utiliza integrações relacionadas ao ecossistema do Instagram/Meta para obtenção dos dados da conta.

Entre os recursos utilizados durante o desenvolvimento estão:

* Instagram API
* Instagram Graph API
* Meta Graph API
* Endpoints HTTP para consulta dos dados necessários

> ⚠️ Alguns endpoints exigem autenticação, Access Token, permissões específicas e uma conta compatível com os recursos da API.

## 🔄 Funcionamento

O fluxo básico do projeto é:

```text
ESP32 liga
   ↓
Conecta ao Wi-Fi
   ↓
Conecta à API
   ↓
Faz uma requisição HTTP
   ↓
Recebe JSON
   ↓
Processa os dados
   ↓
Atualiza o display
   ↓
Aguarda próxima atualização
   ↓
Repete
```

## 🧰 Hardware

Para reproduzir o projeto você precisará de:

* ESP32
* Display TFT Touch 2.8"
* Cabo USB
* Computador
* Conexão Wi-Fi


## 💻 Software

O projeto pode ser desenvolvido utilizando:

* Arduino IDE
* ESP32 Arduino Core
* Bibliotecas do display
* Bibliotecas para Touch
* WiFi
* HTTPClient
* ArduinoJson


## 🔐 Dados sensíveis

Nunca publique no GitHub informações como:

```text
ACCESS_TOKEN
CLIENT_SECRET
APP_SECRET
Senhas de Wi-Fi
Tokens permanentes
Credenciais de APIs
```


## 🎯 Objetivo educacional

Mais do que criar um contador de seguidores, este projeto serve para estudar conceitos importantes utilizados em sistemas reais:

* Internet das Coisas — IoT
* APIs
* HTTP
* JSON
* Microcontroladores
* Wi-Fi
* Sistemas embarcados
* Interfaces gráficas
* Integração entre hardware e software


## 👨‍💻 Autor

**Lucas Carvalho — LC Sistemas**

Projetos de programação, eletrônica, IoT, Arduino, ESP32 e cultura Maker.

📸 Instagram: **@br.lcsistemas**

## 📜 Licença

Projeto disponibilizado para fins **educacionais e de aprendizado**.

Você pode estudar, modificar e adaptar o código para criar seus próprios projetos.

---

⭐ Se este projeto te ajudou, deixe uma estrela no repositório.

🔧 **Aprenda. Teste. Modifique. Crie.**

**LC Sistemas — Tecnologia na prática.**
