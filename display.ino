/*
  ==========================================================
          LC SISTEMAS - SMART DISPLAY
  ==========================================================

  ESP32-2432S028R / CYD
  ILI9341 320x240
  Touch XPT2046

  TELAS:

  1 - Instagram
  2 - Relogio
  3 - Bitcoin
  4 - Dispositivos IoT
  5 - Clima
  6 - Nascer / Por do Sol
  7 - Cotacoes
  8 - Comentarios recentes

  APIs:

  Instagram Graph API
  CoinGecko
  Open-Meteo
  Sunrise-Sunset.org v2
  Frankfurter v2
  Instagram Comments API

  ==========================================================
*/


#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LovyanGFX.hpp>
#include <time.h>


// ======================================================
// LOCALIZACAO
//
// ALTERE AQUI PARA SUA LOCALIZACAO
// ======================================================

const double LATITUDE =
  -10.68;

const double LONGITUDE =
  -37.86;


// Nome exibido no clima

const char* NOME_LOCAL =
  "YOUR CITY";


// ======================================================
// INSTAGRAM
// ======================================================

const char* INSTAGRAM_USER_ID =
  "26234239";

const char* INSTAGRAM_USERNAME =
  "br.lcsistemas";

const char* INSTAGRAM_TOKEN =
  "your_token_here";


// ======================================================
// INTERVALOS
// ======================================================

// Instagram - 40 minutos

const unsigned long INTERVALO_INSTAGRAM =
  40UL * 60UL * 1000UL;


// Bitcoin - 5 minutos

const unsigned long INTERVALO_BITCOIN =
  5UL * 60UL * 1000UL;


// Clima - 10 minutos

const unsigned long INTERVALO_CLIMA =
  10UL * 60UL * 1000UL;


// Sol - 6 horas
// Os horarios do dia praticamente nao precisam
// ser consultados constantemente.

const unsigned long INTERVALO_SOL =
  6UL * 60UL * 60UL * 1000UL;


// Cotacoes - 6 horas
// Frankfurter trabalha com taxas de referencia,
// nao trading em tempo real.

const unsigned long INTERVALO_COTACOES =
  6UL * 60UL * 60UL * 1000UL;


// Comentarios Instagram - 5 minutos

const unsigned long INTERVALO_COMENTARIOS =
  5UL * 60UL * 1000UL;


// Troca de tela - 1 minuto

const unsigned long INTERVALO_TELA =
  60UL * 1000UL;


// ======================================================
// TFT
// ======================================================

#define TFT_SCLK 14
#define TFT_MOSI 13
#define TFT_MISO 12

#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  -1

#define TFT_BL   21


// ======================================================
// TOUCH
// ======================================================

#define TOUCH_CS   33
#define TOUCH_IRQ  36

#define TOUCH_SCLK 25
#define TOUCH_MOSI 32
#define TOUCH_MISO 39


// ======================================================
// LOVYANGFX
// ======================================================

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Touch_XPT2046 _touch_instance;

public:

  LGFX()
  {

    // ==================================================
    // DISPLAY
    // ==================================================

    {
      auto cfg =
        _bus_instance.config();

      cfg.spi_host =
        SPI2_HOST;

      cfg.spi_mode =
        0;

      cfg.freq_write =
        40000000;

      cfg.freq_read =
        16000000;

      cfg.spi_3wire =
        false;

      cfg.use_lock =
        true;

      cfg.dma_channel =
        SPI_DMA_CH_AUTO;

      cfg.pin_sclk =
        TFT_SCLK;

      cfg.pin_mosi =
        TFT_MOSI;

      cfg.pin_miso =
        TFT_MISO;

      cfg.pin_dc =
        TFT_DC;

      _bus_instance.config(
        cfg
      );

      _panel_instance.setBus(
        &_bus_instance
      );
    }


    // ==================================================
    // PAINEL
    // ==================================================

    {
      auto cfg =
        _panel_instance.config();

      cfg.pin_cs =
        TFT_CS;

      cfg.pin_rst =
        TFT_RST;

      cfg.pin_busy =
        -1;

      cfg.panel_width =
        240;

      cfg.panel_height =
        320;

      cfg.offset_x =
        0;

      cfg.offset_y =
        0;

      cfg.offset_rotation =
        0;

      cfg.readable =
        true;

      cfg.invert =
        false;

      cfg.rgb_order =
        false;

      cfg.dlen_16bit =
        false;

      cfg.bus_shared =
        false;

      _panel_instance.config(
        cfg
      );
    }


    // ==================================================
    // TOUCH
    // ==================================================

    {
      auto cfg =
        _touch_instance.config();

      cfg.x_min =
        200;

      cfg.x_max =
        3900;

      cfg.y_min =
        200;

      cfg.y_max =
        3900;

      cfg.pin_int =
        TOUCH_IRQ;

      cfg.bus_shared =
        false;

      cfg.offset_rotation =
        0;

      cfg.spi_host =
        VSPI_HOST;

      cfg.freq =
        1000000;

      cfg.pin_sclk =
        TOUCH_SCLK;

      cfg.pin_mosi =
        TOUCH_MOSI;

      cfg.pin_miso =
        TOUCH_MISO;

      cfg.pin_cs =
        TOUCH_CS;

      _touch_instance.config(
        cfg
      );

      _panel_instance.setTouch(
        &_touch_instance
      );
    }


    setPanel(
      &_panel_instance
    );
  }
};


LGFX lcd;


// ======================================================
// VARIAVEIS INSTAGRAM
// ======================================================

long seguidoresAtual =
  0;

long seguidoresAnterior =
  0;

bool primeiraLeitura =
  true;

String ultimaAtualizacaoInstagram =
  "--/-- --:--";


// ======================================================
// BITCOIN
// ======================================================

double bitcoinUSD =
  0;

double bitcoinBRL =
  0;

String ultimaAtualizacaoBitcoin =
  "--/-- --:--";


// ======================================================
// CLIMA
// ======================================================

float temperatura =
  0;

float sensacaoTermica =
  0;

int umidade =
  0;

float chuvaAgora =
  0;

int probabilidadeChuva =
  0;

int codigoClima =
  -1;

String ultimaAtualizacaoClima =
  "--/-- --:--";


// ======================================================
// SOL
// ======================================================

String horarioNascerSol =
  "--:--";

String horarioPorSol =
  "--:--";

String duracaoDia =
  "--h--";

String ultimaAtualizacaoSol =
  "--/-- --:--";


// ======================================================
// COTACOES
// ======================================================

double dolarBRL =
  0;

double euroBRL =
  0;

String dataCotacao =
  "--/--";

String ultimaAtualizacaoCotacao =
  "--/-- --:--";


// ======================================================
// COMENTARIOS INSTAGRAM
// ======================================================

struct ComentarioInstagram
{
  String id;
  String texto;
  String username;
  String timestamp;
};

const int MAX_COMENTARIOS = 20;
const int MIDIAS_PARA_COMENTARIOS = 5;
const int COMENTARIOS_POR_MIDIA = 15;
const int COMENTARIOS_VISIVEIS = 3;

ComentarioInstagram comentarios[MAX_COMENTARIOS];
int totalComentarios = 0;
int scrollComentarios = 0;

String ultimaAtualizacaoComentarios =
  "--/-- --:--";


// ======================================================
// IOT
// ======================================================

struct DispositivoIoT
{
  String nome;
  bool online;
};


DispositivoIoT dispositivos[] =
{
  {"Portao garagem", true},
  {"Alimentador Aquiles", true},
  {"Alimentador Chambelo", false},
  {"Luz frente", true},
  {"Computador", true},
  {"Alarme porta vidro", true}
};


const int TOTAL_DISPOSITIVOS =
  sizeof(dispositivos) /
  sizeof(dispositivos[0]);


// ======================================================
// LISTA IOT
// ======================================================

const int IOT_LISTA_Y =
  58;

const int IOT_LISTA_ALTURA =
  152;

const int ALTURA_ITEM_IOT =
  38;

const int ITENS_VISIVEIS_IOT =
  4;

int scrollIoT =
  0;


// ======================================================
// TELAS
// ======================================================

enum Tela
{
  TELA_INSTAGRAM = 0,
  TELA_RELOGIO = 1,
  TELA_BITCOIN = 2,
  TELA_IOT = 3,
  TELA_CLIMA = 4,
  TELA_SOL = 5,
  TELA_COTACOES = 6,
  TELA_COMENTARIOS = 7
};


const int TOTAL_TELAS =
  8;


int telaAtual =
  TELA_INSTAGRAM;


// ======================================================
// TIMERS
// ======================================================

unsigned long ultimaConsultaInstagram =
  0;

unsigned long ultimaConsultaBitcoin =
  0;

unsigned long ultimaConsultaClima =
  0;

unsigned long ultimaConsultaSol =
  0;

unsigned long ultimaConsultaCotacoes =
  0;

unsigned long ultimaConsultaComentarios =
  0;

unsigned long ultimaTrocaTela =
  0;

unsigned long ultimaAtualizacaoRelogio =
  0;


// ======================================================
// TOUCH
// ======================================================

bool touchAnterior =
  false;

int touchInicioX =
  0;

int touchInicioY =
  0;

int touchUltimoX =
  0;

int touchUltimoY =
  0;

unsigned long touchInicioTempo =
  0;


// ======================================================
// CORES
// ======================================================

uint16_t COR_CARD;
uint16_t COR_CARD_BORDA;
uint16_t COR_AZUL_CLARO;
uint16_t COR_AZUL_RELOGIO;
uint16_t COR_BITCOIN;

uint16_t COR_ONLINE;
uint16_t COR_OFFLINE;
uint16_t COR_TEXTO_IOT;
uint16_t COR_CINZA_IOT;

uint16_t COR_CLIMA;
uint16_t COR_SOL;
uint16_t COR_COTACAO;
uint16_t COR_COMENTARIOS;


// ======================================================
// FORMATAR NUMERO
// ======================================================

String formatarNumero(long numero)
{

  String entrada =
    String(numero);

  String saida =
    "";

  int contador =
    0;


  for (
    int i = entrada.length() - 1;
    i >= 0;
    i--
  )
  {

    saida =
      entrada[i] +
      saida;

    contador++;


    if (
      contador == 3 &&
      i > 0
    )
    {

      saida =
        "." +
        saida;

      contador =
        0;
    }
  }


  return saida;
}


// ======================================================
// FORMATAR DINHEIRO
// ======================================================

String formatarDinheiro(
  double valor
)
{

  unsigned long inteiro =
    (unsigned long)valor;


  int centavos =
    (int)(
      (valor - inteiro) *
      100.0 +
      0.5
    );


  if (
    centavos >= 100
  )
  {

    inteiro++;

    centavos =
      0;
  }


  String parteInteira =
    formatarNumero(
      inteiro
    );


  String parteCentavos =
    String(centavos);


  if (
    centavos < 10
  )
  {

    parteCentavos =
      "0" +
      parteCentavos;
  }


  return
    parteInteira +
    "," +
    parteCentavos;
}


// ======================================================
// DATA E HORA
// ======================================================

String obterDataHora()
{

  struct tm timeinfo;


  if (
    !getLocalTime(
      &timeinfo,
      1000
    )
  )
  {

    return "--/-- --:--";
  }


  char buffer[20];


  strftime(
    buffer,
    sizeof(buffer),
    "%d/%m %H:%M",
    &timeinfo
  );


  return String(buffer);
}


// ======================================================
// DIA DA SEMANA
// ======================================================

String obterDiaSemana(
  int dia
)
{

  switch (dia)
  {
    case 0: return "Dom";
    case 1: return "Seg";
    case 2: return "Ter";
    case 3: return "Qua";
    case 4: return "Qui";
    case 5: return "Sex";
    case 6: return "Sab";
  }


  return "";
}


// ======================================================
// EXTRAIR HH:MM DE ISO8601
//
// 2026-09-18T05:25:13-03:00
// ->
// 05:25
// ======================================================

String extrairHoraISO(
  String valor
)
{

  int posicaoT =
    valor.indexOf('T');


  if (
    posicaoT >= 0 &&
    valor.length() >= posicaoT + 6
  )
  {

    return valor.substring(
      posicaoT + 1,
      posicaoT + 6
    );
  }


  return "--:--";
}


// ======================================================
// FORMATAR DURACAO DO DIA
// ======================================================

String formatarDuracaoDia(
  long segundos
)
{

  if (
    segundos <= 0
  )
  {

    return "--h--";
  }


  int horas =
    segundos / 3600;


  int minutos =
    (segundos % 3600) /
    60;


  char buffer[20];


  snprintf(
    buffer,
    sizeof(buffer),
    "%dh %02dmin",
    horas,
    minutos
  );


  return String(buffer);
}


// ======================================================
// DESCRICAO CLIMA
// WMO WEATHER CODE
// ======================================================

String descricaoClima(
  int codigo
)
{

  if (codigo == 0)
    return "Ceu limpo";

  if (
    codigo == 1 ||
    codigo == 2
  )
    return "Parcialmente nublado";

  if (codigo == 3)
    return "Nublado";

  if (
    codigo == 45 ||
    codigo == 48
  )
    return "Neblina";

  if (
    codigo >= 51 &&
    codigo <= 57
  )
    return "Garoa";

  if (
    codigo >= 61 &&
    codigo <= 67
  )
    return "Chuva";

  if (
    codigo >= 71 &&
    codigo <= 77
  )
    return "Neve";

  if (
    codigo >= 80 &&
    codigo <= 82
  )
    return "Pancadas de chuva";

  if (
    codigo >= 95
  )
    return "Trovoadas";


  return "Condicao variavel";
}


// ======================================================
// INTERPOLAR COR
// ======================================================

uint16_t interpolarCor(
  uint8_t r1,
  uint8_t g1,
  uint8_t b1,

  uint8_t r2,
  uint8_t g2,
  uint8_t b2,

  float fator
)
{

  uint8_t r =
    r1 +
    ((r2 - r1) * fator);

  uint8_t g =
    g1 +
    ((g2 - g1) * fator);

  uint8_t b =
    b1 +
    ((b2 - b1) * fator);


  return lcd.color565(
    r,
    g,
    b
  );
}


// ======================================================
// FUNDO INSTAGRAM
// ======================================================

void desenharFundoInstagram()
{

  for (
    int y = 0;
    y < 240;
    y++
  )
  {

    float fator =
      (float)y /
      240.0;


    uint16_t cor =
      interpolarCor(
        0, 22, 68,
        0, 116, 210,
        fator
      );


    lcd.drawFastHLine(
      0,
      y,
      320,
      cor
    );
  }
}


// ======================================================
// INDICADORES
// ======================================================

void desenharIndicadores(
  uint16_t corAtiva,
  uint16_t corInativa
)
{

  int distancia =
    16;

  int larguraTotal =
    (TOTAL_TELAS - 1) *
    distancia;

  int inicioX =
    160 -
    (larguraTotal / 2);


  for (
    int i = 0;
    i < TOTAL_TELAS;
    i++
  )
  {

    int x =
      inicioX +
      (i * distancia);


    if (
      i == telaAtual
    )
    {

      lcd.fillCircle(
        x,
        228,
        4,
        corAtiva
      );
    }

    else
    {

      lcd.drawCircle(
        x,
        228,
        4,
        corInativa
      );
    }
  }
}


// ======================================================
// INSTAGRAM
// ======================================================

void desenharTelaInstagram()
{

  desenharFundoInstagram();


  lcd.setTextDatum(
    middle_center
  );


  lcd.setFont(
    &fonts::Font4
  );

  lcd.setTextSize(1);

  lcd.setTextColor(
    TFT_WHITE
  );


  lcd.drawString(
    "INSTAGRAM",
    160,
    24
  );


  lcd.setFont(
    &fonts::Font2
  );

  lcd.setTextColor(
    COR_AZUL_CLARO
  );


  lcd.drawString(
    "@br.lcsistemas",
    160,
    51
  );


  // CARD

  lcd.fillRoundRect(
    20,
    70,
    280,
    120,
    18,
    COR_CARD
  );


  lcd.drawRoundRect(
    20,
    70,
    280,
    120,
    18,
    COR_CARD_BORDA
  );


  if (
    seguidoresAtual > 0
  )
  {

    String numero =
      formatarNumero(
        seguidoresAtual
      );


    lcd.setFont(
      &fonts::Font4
    );

    lcd.setTextSize(2);


    lcd.setTextColor(
      lcd.color565(
        0,
        35,
        90
      )
    );


    lcd.drawString(
      numero,
      163,
      120
    );


    lcd.setTextColor(
      TFT_WHITE
    );


    lcd.drawString(
      numero,
      160,
      117
    );


    lcd.setFont(
      &fonts::Font2
    );

    lcd.setTextSize(1);

    lcd.setTextColor(
      COR_AZUL_CLARO
    );


    lcd.drawString(
      "SEGUIDORES",
      160,
      168
    );
  }

  else
  {

    lcd.setFont(
      &fonts::Font2
    );

    lcd.setTextSize(1);

    lcd.setTextColor(
      TFT_WHITE
    );


    lcd.drawString(
      "Carregando...",
      160,
      130
    );
  }


  lcd.setFont(
    &fonts::Font0
  );

  lcd.setTextSize(1);

  lcd.setTextColor(
    TFT_WHITE
  );


  lcd.drawString(
    "Atualizado: " +
    ultimaAtualizacaoInstagram,
    160,
    204
  );


  desenharIndicadores(
    TFT_WHITE,
    COR_AZUL_CLARO
  );
}


// ======================================================
// RELOGIO
// ======================================================

void desenharTelaRelogio()
{

  lcd.fillScreen(
    TFT_WHITE
  );


  struct tm timeinfo;


  lcd.setTextDatum(
    middle_center
  );


  if (
    !getLocalTime(
      &timeinfo,
      1000
    )
  )
  {

    lcd.setFont(
      &fonts::Font2
    );

    lcd.setTextColor(
      COR_AZUL_RELOGIO
    );


    lcd.drawString(
      "Sincronizando horario...",
      160,
      120
    );


    return;
  }


  char data[10];


  snprintf(
    data,
    sizeof(data),
    "%02d/%02d",
    timeinfo.tm_mday,
    timeinfo.tm_mon + 1
  );


  String linhaData =
    String(data) +
    " " +
    obterDiaSemana(
      timeinfo.tm_wday
    );


  lcd.setFont(
    &fonts::Font4
  );

  lcd.setTextSize(1);

  lcd.setTextColor(
    COR_AZUL_RELOGIO
  );


  lcd.drawString(
    linhaData,
    160,
    65
  );


  char horario[10];


  snprintf(
    horario,
    sizeof(horario),
    "%02d:%02d",
    timeinfo.tm_hour,
    timeinfo.tm_min
  );


  lcd.setFont(
    &fonts::Font4
  );

  lcd.setTextSize(2);


  lcd.drawString(
    horario,
    160,
    130
  );


  lcd.setFont(
    &fonts::Font0
  );

  lcd.setTextSize(1);


  lcd.drawString(
    "LC SISTEMAS",
    160,
    190
  );


  desenharIndicadores(
    COR_AZUL_RELOGIO,
    lcd.color565(
      170,
      190,
      220
    )
  );
}


// ======================================================
// COMENTARIOS INSTAGRAM
// Busca as 5 midias mais recentes e consulta os
// comentarios de cada uma.
// Atualizacao: a cada 5 minutos.
// ======================================================

bool comentarioJaExiste(String id)
{
  for (int i = 0; i < totalComentarios; i++)
  {
    if (comentarios[i].id == id)
      return true;
  }

  return false;
}


void ordenarComentariosPorData()
{
  for (int i = 0; i < totalComentarios - 1; i++)
  {
    for (int j = i + 1; j < totalComentarios; j++)
    {
      // ISO 8601 pode ser comparado como texto neste caso.
      // Maior timestamp = comentario mais recente.
      if (comentarios[j].timestamp > comentarios[i].timestamp)
      {
        ComentarioInstagram temp = comentarios[i];
        comentarios[i] = comentarios[j];
        comentarios[j] = temp;
      }
    }
  }
}


bool buscarComentariosDaMidia(String mediaId)
{
  String url =
    "https://graph.instagram.com/v25.0/";

  url += mediaId;
  url +=
    "/comments?fields=id,text,username,timestamp&limit=";
  url += String(COMENTARIOS_POR_MIDIA);
  url += "&access_token=";
  url += INSTAGRAM_TOKEN;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  if (!http.begin(client, url))
    return false;

  http.setTimeout(20000);

  int codigo = http.GET();
  String resposta = http.getString();
  http.end();

  Serial.print("Comentarios HTTP: ");
  Serial.println(codigo);

  if (codigo != 200)
  {
    // Nao imprime a URL para nao expor o token.
    Serial.println("Falha ao consultar comentarios da midia.");
    return false;
  }

  JsonDocument doc;

  DeserializationError erro =
    deserializeJson(doc, resposta);

  if (erro)
  {
    Serial.print("Erro JSON comentarios: ");
    Serial.println(erro.c_str());
    return false;
  }

  JsonArray dados =
    doc["data"].as<JsonArray>();

  for (JsonObject item : dados)
  {
    if (totalComentarios >= MAX_COMENTARIOS)
      break;

    String id = item["id"] | "";

    if (id.length() == 0 || comentarioJaExiste(id))
      continue;

    comentarios[totalComentarios].id = id;
    comentarios[totalComentarios].texto =
      item["text"] | "";

    // Alguns comentarios nao retornam username.
    // Nesse caso deixamos vazio e a tela mostra apenas o texto.
    comentarios[totalComentarios].username =
      item["username"] | "";

    comentarios[totalComentarios].timestamp =
      item["timestamp"] | "";

    totalComentarios++;
  }

  return true;
}


bool buscarComentariosRecentes()
{
  if (WiFi.status() != WL_CONNECTED)
    return false;

  if (strlen(INSTAGRAM_TOKEN) < 20)
  {
    Serial.println("Configure INSTAGRAM_TOKEN para comentarios.");
    return false;
  }

  String url =
    "https://graph.instagram.com/v25.0/";

  url += INSTAGRAM_USER_ID;
  url +=
    "/media?fields=id,timestamp&limit=";
  url += String(MIDIAS_PARA_COMENTARIOS);
  url += "&access_token=";
  url += INSTAGRAM_TOKEN;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  if (!http.begin(client, url))
    return false;

  http.setTimeout(20000);

  int codigo = http.GET();
  String resposta = http.getString();
  http.end();

  Serial.print("Midias para comentarios HTTP: ");
  Serial.println(codigo);

  if (codigo != 200)
  {
    Serial.println("Falha ao buscar midias recentes.");
    return false;
  }

  JsonDocument doc;

  DeserializationError erro =
    deserializeJson(doc, resposta);

  if (erro)
  {
    Serial.print("Erro JSON midias: ");
    Serial.println(erro.c_str());
    return false;
  }

  // Monta uma lista nova a cada atualizacao.
  totalComentarios = 0;
  scrollComentarios = 0;

  JsonArray midias =
    doc["data"].as<JsonArray>();

  int consultadas = 0;

  for (JsonObject midia : midias)
  {
    if (consultadas >= MIDIAS_PARA_COMENTARIOS)
      break;

    String mediaId = midia["id"] | "";

    if (mediaId.length() == 0)
      continue;

    buscarComentariosDaMidia(mediaId);
    consultadas++;

    // Evita varias conexoes HTTPS exatamente no mesmo instante.
    delay(150);
  }

  ordenarComentariosPorData();

  ultimaAtualizacaoComentarios =
    obterDataHora();

  Serial.print("Comentarios carregados: ");
  Serial.println(totalComentarios);

  if (telaAtual == TELA_COMENTARIOS)
    desenharTelaComentarios();

  return true;
}


// ======================================================
// BITCOIN
// ======================================================

void desenharTelaBitcoin()
{

  lcd.fillScreen(
    TFT_BLACK
  );


  lcd.setTextDatum(
    middle_center
  );


  lcd.setFont(
    &fonts::Font4
  );

  lcd.setTextSize(1);

  lcd.setTextColor(
    COR_BITCOIN
  );


  lcd.drawString(
    "BITCOIN",
    160,
    23
  );


  lcd.setFont(
    &fonts::Font2
  );


  lcd.drawString(
    "BTC / USD",
    160,
    58
  );


  String textoUSD =
    bitcoinUSD > 0
    ?
    "$ " +
    formatarDinheiro(bitcoinUSD)
    :
    "Carregando...";


  lcd.setFont(
    &fonts::Font4
  );

  lcd.setTextColor(
    TFT_WHITE
  );


  lcd.drawString(
    textoUSD,
    160,
    87
  );


  lcd.drawFastHLine(
    65,
    111,
    190,
    lcd.color565(
      50,
      50,
      50
    )
  );


  lcd.setFont(
    &fonts::Font2
  );

  lcd.setTextColor(
    COR_BITCOIN
  );


  lcd.drawString(
    "BTC / BRL",
    160,
    132
  );


  String textoBRL =
    bitcoinBRL > 0
    ?
    "R$ " +
    formatarDinheiro(bitcoinBRL)
    :
    "Carregando...";


  lcd.setFont(
    &fonts::Font4
  );

  lcd.setTextColor(
    TFT_WHITE
  );


  lcd.drawString(
    textoBRL,
    160,
    161
  );


  lcd.setFont(
    &fonts::Font0
  );

  lcd.setTextColor(
    lcd.color565(
      150,
      150,
      150
    )
  );


  lcd.drawString(
    "Atualizado: " +
    ultimaAtualizacaoBitcoin,
    160,
    202
  );


  desenharIndicadores(
    TFT_WHITE,
    lcd.color565(
      100,
      100,
      100
    )
  );
}


// ======================================================
// IOT MOCK
// ======================================================

void atualizarDispositivosIoT()
{

  dispositivos[0].online = true;
  dispositivos[1].online = true;
  dispositivos[2].online = false;
  dispositivos[3].online = true;
  dispositivos[4].online = true;
  dispositivos[5].online = true;
}


// ======================================================
// ITEM IOT
// ======================================================

void desenharDispositivoIoT(
  int indice,
  int y
)
{

  if (
    y < IOT_LISTA_Y + 10 ||
    y >
      IOT_LISTA_Y +
      IOT_LISTA_ALTURA -
      10
  )
  {

    return;
  }


  bool online =
    dispositivos[indice].online;


  uint16_t corStatus =
    online
    ?
    COR_ONLINE
    :
    COR_OFFLINE;


  lcd.fillCircle(
    22,
    y,
    7,
    corStatus
  );


  lcd.setTextDatum(
    middle_left
  );


  lcd.setFont(
    &fonts::Font2
  );

  lcd.setTextColor(
    COR_TEXTO_IOT
  );


  lcd.drawString(
    dispositivos[indice].nome,
    39,
    y
  );


  lcd.setTextDatum(
    middle_right
  );


  lcd.setTextColor(
    corStatus
  );


  lcd.drawString(
    online ? "ON" : "OFF",
    298,
    y
  );


  lcd.drawFastHLine(
    14,
    y + 18,
    290,
    lcd.color565(
      225,
      230,
      235
    )
  );
}


// ======================================================
// LISTA IOT
// ======================================================

void desenharListaIoT()
{

  lcd.fillRect(
    0,
    IOT_LISTA_Y,
    320,
    IOT_LISTA_ALTURA,
    TFT_WHITE
  );


  int yInicial =
    IOT_LISTA_Y + 18;


  for (
    int i = 0;
    i < TOTAL_DISPOSITIVOS;
    i++
  )
  {

    int y =
      yInicial +
      (
        (i - scrollIoT) *
        ALTURA_ITEM_IOT
      );


    desenharDispositivoIoT(
      i,
      y
    );
  }


  // BARRA DE ROLAGEM

  if (
    TOTAL_DISPOSITIVOS >
    ITENS_VISIVEIS_IOT
  )
  {

    int alturaTotal =
      IOT_LISTA_ALTURA - 8;


    int alturaIndicador =
      (
        alturaTotal *
        ITENS_VISIVEIS_IOT
      ) /
      TOTAL_DISPOSITIVOS;


    if (
      alturaIndicador < 20
    )
    {

      alturaIndicador =
        20;
    }


    int maxScroll =
      TOTAL_DISPOSITIVOS -
      ITENS_VISIVEIS_IOT;


    int posicao =
      0;


    if (
      maxScroll > 0
    )
    {

      posicao =
        (
          (
            alturaTotal -
            alturaIndicador
          ) *
          scrollIoT
        ) /
        maxScroll;
    }


    lcd.fillRoundRect(
      313,
      IOT_LISTA_Y + 4,
      4,
      alturaTotal,
      2,
      lcd.color565(
        225,
        230,
        235
      )
    );


    lcd.fillRoundRect(
      313,
      IOT_LISTA_Y +
      4 +
      posicao,
      4,
      alturaIndicador,
      2,
      COR_AZUL_RELOGIO
    );
  }
}


// ======================================================
// TELA IOT
// ======================================================

void desenharTelaIoT()
{

  lcd.fillScreen(
    TFT_WHITE
  );


  lcd.setTextDatum(
    middle_center
  );


  lcd.setFont(
    &fonts::Font4
  );

  lcd.setTextColor(
    COR_AZUL_RELOGIO
  );


  lcd.drawString(
    "DISPOSITIVOS IoT",
    160,
    20
  );


  lcd.setFont(
    &fonts::Font0
  );

  lcd.setTextColor(
    COR_CINZA_IOT
  );


  lcd.drawString(
    String(TOTAL_DISPOSITIVOS) +
    " dispositivos",
    160,
    43
  );


  desenharListaIoT();


  desenharIndicadores(
    COR_AZUL_RELOGIO,
    lcd.color565(
      170,
      190,
      220
    )
  );
}


// ======================================================
// TELA CLIMA
// ======================================================

void desenharTelaClima()
{

  lcd.fillScreen(
    lcd.color565(
      225,
      244,
      255
    )
  );


  lcd.setTextDatum(
    middle_center
  );


  lcd.setFont(
    &fonts::Font4
  );

  lcd.setTextSize(1);

  lcd.setTextColor(
    COR_CLIMA
  );


  lcd.drawString(
    "CLIMA",
    160,
    19
  );


  lcd.setFont(
    &fonts::Font0
  );


  lcd.drawString(
    NOME_LOCAL,
    160,
    40
  );


  // TEMPERATURA

  lcd.setFont(
    &fonts::Font4
  );

  lcd.setTextSize(2);


  String temp =
    String(
      temperatura,
      1
    ) +
    " C";


  lcd.drawString(
    temp,
    160,
    78
  );


  // CONDICAO

  lcd.setFont(
    &fonts::Font2
  );

  lcd.setTextSize(1);


  lcd.drawString(
    descricaoClima(
      codigoClima
    ),
    160,
    113
  );


  // INFORMACOES

  lcd.setTextDatum(
    middle_left
  );


  lcd.setFont(
    &fonts::Font2
  );


  lcd.setTextColor(
    lcd.color565(
      40,
      60,
      80
    )
  );


  lcd.drawString(
    "Umidade",
    25,
    145
  );


  lcd.drawString(
    "Sensacao",
    25,
    169
  );


  lcd.drawString(
    "Chuva agora",
    25,
    193
  );


  lcd.setTextDatum(
    middle_right
  );


  lcd.setTextColor(
    COR_CLIMA
  );


  lcd.drawString(
    String(umidade) + "%",
    295,
    145
  );


  lcd.drawString(
    String(
      sensacaoTermica,
      1
    ) +
    " C",
    295,
    169
  );


  lcd.drawString(
    String(
      chuvaAgora,
      1
    ) +
    " mm / " +
    String(
      probabilidadeChuva
    ) +
    "%",
    295,
    193
  );


  desenharIndicadores(
    COR_CLIMA,
    lcd.color565(
      150,
      190,
      210
    )
  );
}


// ======================================================
// TELA SOL
// ======================================================

void desenharTelaSol()
{

  lcd.fillScreen(
    lcd.color565(
      255,
      248,
      225
    )
  );


  lcd.setTextDatum(
    middle_center
  );


  lcd.setFont(
    &fonts::Font4
  );

  lcd.setTextSize(1);

  lcd.setTextColor(
    COR_SOL
  );


  lcd.drawString(
    "SOL",
    160,
    22
  );


  lcd.setFont(
    &fonts::Font0
  );


  lcd.drawString(
    NOME_LOCAL,
    160,
    44
  );


  // NASCER

  lcd.setFont(
    &fonts::Font2
  );


  lcd.drawString(
    "NASCER DO SOL",
    160,
    76
  );


  lcd.setFont(
    &fonts::Font4
  );

  lcd.setTextSize(2);


  lcd.drawString(
    horarioNascerSol,
    160,
    105
  );


  // POR DO SOL

  lcd.setFont(
    &fonts::Font2
  );

  lcd.setTextSize(1);


  lcd.drawString(
    "POR DO SOL",
    160,
    145
  );


  lcd.setFont(
    &fonts::Font4
  );

  lcd.setTextSize(2);


  lcd.drawString(
    horarioPorSol,
    160,
    174
  );


  // DURACAO

  lcd.setFont(
    &fonts::Font0
  );

  lcd.setTextSize(1);


  lcd.drawString(
    "Duracao do dia: " +
    duracaoDia,
    160,
    205
  );


  desenharIndicadores(
    COR_SOL,
    lcd.color565(
      210,
      180,
      120
    )
  );
}


// ======================================================
// TELA COTACOES
// ======================================================

void desenharTelaCotacoes()
{

  lcd.fillScreen(
    lcd.color565(
      242,
      248,
      244
    )
  );


  lcd.setTextDatum(
    middle_center
  );


  lcd.setFont(
    &fonts::Font4
  );

  lcd.setTextSize(1);

  lcd.setTextColor(
    COR_COTACAO
  );


  lcd.drawString(
    "COTACOES",
    160,
    22
  );


  lcd.setFont(
    &fonts::Font0
  );


  lcd.drawString(
    "Valores de referencia",
    160,
    44
  );


  // DOLAR

  lcd.setFont(
    &fonts::Font2
  );


  lcd.drawString(
    "DOLAR / REAL",
    160,
    76
  );


  lcd.setFont(
    &fonts::Font4
  );

  lcd.setTextSize(1);


  String dolarTexto =
    dolarBRL > 0
    ?
    "R$ " +
    String(
      dolarBRL,
      4
    )
    :
    "Carregando...";


  lcd.drawString(
    dolarTexto,
    160,
    104
  );


  // EURO

  lcd.setFont(
    &fonts::Font2
  );


  lcd.drawString(
    "EURO / REAL",
    160,
    140
  );


  lcd.setFont(
    &fonts::Font4
  );


  String euroTexto =
    euroBRL > 0
    ?
    "R$ " +
    String(
      euroBRL,
      4
    )
    :
    "Carregando...";


  lcd.drawString(
    euroTexto,
    160,
    168
  );


  lcd.setFont(
    &fonts::Font0
  );


  lcd.drawString(
    "Referencia: " +
    dataCotacao,
    160,
    202
  );


  desenharIndicadores(
    COR_COTACAO,
    lcd.color565(
      160,
      195,
      170
    )
  );
}


// ======================================================
// TEXTO PARA DISPLAY
// Remove emojis e converte acentos comuns para ASCII
// para manter a leitura nas fontes internas do display.
// ======================================================

String textoSeguroDisplay(String texto)
{
  String saida = "";

  for (int i = 0; i < texto.length(); i++)
  {
    uint8_t c = (uint8_t)texto[i];

    if (c < 128)
    {
      if (c == '\n' || c == '\r' || c == '\t')
        saida += ' ';
      else
        saida += (char)c;

      continue;
    }

    // UTF-8 portugues mais comum
    if (c == 0xC3 && i + 1 < texto.length())
    {
      uint8_t d = (uint8_t)texto[++i];

      switch (d)
      {
        case 0x81: case 0x80: case 0x82: case 0x83: case 0x84: saida += 'A'; break;
        case 0x89: case 0x88: case 0x8A: case 0x8B: saida += 'E'; break;
        case 0x8D: case 0x8C: case 0x8E: case 0x8F: saida += 'I'; break;
        case 0x93: case 0x92: case 0x94: case 0x95: case 0x96: saida += 'O'; break;
        case 0x9A: case 0x99: case 0x9B: case 0x9C: saida += 'U'; break;
        case 0x87: saida += 'C'; break;
        case 0xA1: case 0xA0: case 0xA2: case 0xA3: case 0xA4: saida += 'a'; break;
        case 0xA9: case 0xA8: case 0xAA: case 0xAB: saida += 'e'; break;
        case 0xAD: case 0xAC: case 0xAE: case 0xAF: saida += 'i'; break;
        case 0xB3: case 0xB2: case 0xB4: case 0xB5: case 0xB6: saida += 'o'; break;
        case 0xBA: case 0xB9: case 0xBB: case 0xBC: saida += 'u'; break;
        case 0xA7: saida += 'c'; break;
        default: break;
      }

      continue;
    }

    // Pula o restante de caracteres UTF-8 (emoji etc.)
    if ((c & 0xE0) == 0xC0) i += 1;
    else if ((c & 0xF0) == 0xE0) i += 2;
    else if ((c & 0xF8) == 0xF0) i += 3;
  }

  saida.trim();
  return saida;
}


// ======================================================
// QUEBRAR TEXTO DO COMENTARIO EM ATE 2 LINHAS
// ======================================================

void quebrarComentario(
  String texto,
  String &linha1,
  String &linha2
)
{
  texto = textoSeguroDisplay(texto);

  const int LIMITE = 42;

  if (texto.length() <= LIMITE)
  {
    linha1 = texto;
    linha2 = "";
    return;
  }

  int corte = LIMITE;

  while (corte > 20 && texto[corte] != ' ')
    corte--;

  if (corte <= 20)
    corte = LIMITE;

  linha1 = texto.substring(0, corte);

  int inicio2 = corte;
  while (inicio2 < texto.length() && texto[inicio2] == ' ')
    inicio2++;

  linha2 = texto.substring(inicio2);

  if (linha2.length() > LIMITE)
  {
    linha2 = linha2.substring(0, LIMITE - 3) + "...";
  }
}


// ======================================================
// LISTA DE COMENTARIOS
// ======================================================

void desenharListaComentarios()
{
  const int Y_INICIO = 57;
  const int ALTURA_AREA = 153;
  const int ALTURA_ITEM = 51;

  lcd.fillRect(
    0,
    Y_INICIO,
    320,
    ALTURA_AREA,
    TFT_WHITE
  );

  if (totalComentarios == 0)
  {
    lcd.setTextDatum(middle_center);
    lcd.setFont(&fonts::Font2);
    lcd.setTextColor(COR_CINZA_IOT);
    lcd.drawString("Nenhum comentario encontrado", 160, 125);
    return;
  }

  for (int pos = 0; pos < COMENTARIOS_VISIVEIS; pos++)
  {
    int indice = scrollComentarios + pos;

    if (indice >= totalComentarios)
      break;

    int y = Y_INICIO + (pos * ALTURA_ITEM);

    String linha1;
    String linha2;

    quebrarComentario(
      comentarios[indice].texto,
      linha1,
      linha2
    );

    int textoY = y + 10;

    // Se existe username, mostra @username.
    // Se nao existe, mostra SOMENTE o texto.
    if (comentarios[indice].username.length() > 0)
    {
      String usuario = textoSeguroDisplay(comentarios[indice].username);

      lcd.setTextDatum(top_left);
      lcd.setFont(&fonts::Font0);
      lcd.setTextColor(COR_COMENTARIOS);
      lcd.drawString("@" + usuario, 14, y + 3);

      textoY = y + 18;
    }

    lcd.setTextDatum(top_left);
    lcd.setFont(&fonts::Font0);
    lcd.setTextColor(COR_TEXTO_IOT);

    lcd.drawString(linha1, 14, textoY);

    if (linha2.length() > 0)
      lcd.drawString(linha2, 14, textoY + 13);

    lcd.drawFastHLine(
      12,
      y + ALTURA_ITEM - 2,
      292,
      lcd.color565(225, 230, 235)
    );
  }

  // Barra de rolagem
  if (totalComentarios > COMENTARIOS_VISIVEIS)
  {
    int alturaTotal = ALTURA_AREA - 8;
    int alturaIndicador =
      (alturaTotal * COMENTARIOS_VISIVEIS) /
      totalComentarios;

    if (alturaIndicador < 20)
      alturaIndicador = 20;

    int maxScroll =
      totalComentarios - COMENTARIOS_VISIVEIS;

    int posicao =
      ((alturaTotal - alturaIndicador) * scrollComentarios) /
      maxScroll;

    lcd.fillRoundRect(
      313,
      Y_INICIO + 4,
      4,
      alturaTotal,
      2,
      lcd.color565(225, 230, 235)
    );

    lcd.fillRoundRect(
      313,
      Y_INICIO + 4 + posicao,
      4,
      alturaIndicador,
      2,
      COR_COMENTARIOS
    );
  }
}


// ======================================================
// TELA COMENTARIOS
// ======================================================

void desenharTelaComentarios()
{
  lcd.fillScreen(TFT_WHITE);

  lcd.setTextDatum(middle_center);
  lcd.setFont(&fonts::Font4);
  lcd.setTextSize(1);
  lcd.setTextColor(COR_COMENTARIOS);

  lcd.drawString(
    "COMENTARIOS",
    160,
    18
  );

  lcd.setFont(&fonts::Font0);
  lcd.setTextColor(COR_CINZA_IOT);

  lcd.drawString(
    String(totalComentarios) +
    " recentes | " +
    ultimaAtualizacaoComentarios,
    160,
    42
  );

  desenharListaComentarios();

  desenharIndicadores(
    COR_COMENTARIOS,
    lcd.color565(190, 190, 210)
  );
}


// ======================================================
// DESENHAR TELA ATUAL
// ======================================================

void desenharTelaAtual()
{

  switch (
    telaAtual
  )
  {

    case TELA_INSTAGRAM:
      desenharTelaInstagram();
      break;

    case TELA_RELOGIO:
      desenharTelaRelogio();
      break;

    case TELA_BITCOIN:
      desenharTelaBitcoin();
      break;

    case TELA_IOT:
      atualizarDispositivosIoT();
      desenharTelaIoT();
      break;

    case TELA_CLIMA:
      desenharTelaClima();
      break;

    case TELA_SOL:
      desenharTelaSol();
      break;

    case TELA_COTACOES:
      desenharTelaCotacoes();
      break;

    case TELA_COMENTARIOS:
      desenharTelaComentarios();
      break;
  }
}


// ======================================================
// PROXIMA TELA
// ======================================================

void proximaTela()
{

  telaAtual++;


  if (
    telaAtual >=
    TOTAL_TELAS
  )
  {

    telaAtual =
      0;
  }


  ultimaTrocaTela =
    millis();


  desenharTelaAtual();
}


// ======================================================
// TELA ANTERIOR
// ======================================================

void telaAnterior()
{

  telaAtual--;


  if (
    telaAtual < 0
  )
  {

    telaAtual =
      TOTAL_TELAS - 1;
  }


  ultimaTrocaTela =
    millis();


  desenharTelaAtual();
}


// ======================================================
// TOUCH
// ======================================================

void verificarTouch()
{

  uint16_t x = 0;
  uint16_t y = 0;


  bool tocando =
    lcd.getTouch(
      &x,
      &y
    );


  if (
    tocando &&
    !touchAnterior
  )
  {

    touchInicioX = x;
    touchInicioY = y;

    touchUltimoX = x;
    touchUltimoY = y;

    touchInicioTempo =
      millis();

    touchAnterior =
      true;
  }


  if (
    tocando &&
    touchAnterior
  )
  {

    touchUltimoX = x;
    touchUltimoY = y;
  }


  if (
    !tocando &&
    touchAnterior
  )
  {

    touchAnterior =
      false;


    int diferencaX =
      touchUltimoX -
      touchInicioX;


    int diferencaY =
      touchUltimoY -
      touchInicioY;


    unsigned long duracao =
      millis() -
      touchInicioTempo;


    // ==================================================
    // IOT
    // ==================================================

    if (
      telaAtual ==
      TELA_IOT
    )
    {

      // SWIPE VERTICAL

      if (
        abs(diferencaY) > 35 &&
        abs(diferencaY) >
        abs(diferencaX)
      )
      {

        int maxScroll =
          TOTAL_DISPOSITIVOS -
          ITENS_VISIVEIS_IOT;


        if (
          maxScroll < 0
        )
        {

          maxScroll = 0;
        }


        if (
          diferencaY < 0
        )
        {

          scrollIoT++;


          if (
            scrollIoT >
            maxScroll
          )
          {

            scrollIoT =
              maxScroll;
          }
        }

        else
        {

          scrollIoT--;


          if (
            scrollIoT < 0
          )
          {

            scrollIoT = 0;
          }
        }


        desenharListaIoT();


        ultimaTrocaTela =
          millis();


        return;
      }


      // SWIPE HORIZONTAL

      if (
        abs(diferencaX) > 50 &&
        abs(diferencaX) >
        abs(diferencaY)
      )
      {

        if (
          diferencaX < 0
        )
        {

          proximaTela();
        }

        else
        {

          telaAnterior();
        }


        return;
      }


      // TOQUE

      if (
        duracao < 700 &&
        abs(diferencaX) < 20 &&
        abs(diferencaY) < 20
      )
      {

        proximaTela();

        return;
      }


      return;
    }


    // ==================================================
    // COMENTARIOS - SCROLL VERTICAL
    // ==================================================

    if (
      telaAtual ==
      TELA_COMENTARIOS
    )
    {
      if (
        abs(diferencaY) > 35 &&
        abs(diferencaY) >
        abs(diferencaX)
      )
      {
        int maxScroll =
          totalComentarios -
          COMENTARIOS_VISIVEIS;

        if (maxScroll < 0)
          maxScroll = 0;

        if (diferencaY < 0)
        {
          scrollComentarios++;

          if (scrollComentarios > maxScroll)
            scrollComentarios = maxScroll;
        }
        else
        {
          scrollComentarios--;

          if (scrollComentarios < 0)
            scrollComentarios = 0;
        }

        desenharListaComentarios();
        ultimaTrocaTela = millis();
        return;
      }

      if (
        abs(diferencaX) > 50 &&
        abs(diferencaX) >
        abs(diferencaY)
      )
      {
        if (diferencaX < 0)
          proximaTela();
        else
          telaAnterior();

        return;
      }

      if (
        duracao < 700 &&
        abs(diferencaX) < 20 &&
        abs(diferencaY) < 20
      )
      {
        proximaTela();
        return;
      }

      return;
    }


    // ==================================================
    // OUTRAS TELAS
    // ==================================================

    if (
      abs(diferencaX) > 50 &&
      abs(diferencaX) >
      abs(diferencaY)
    )
    {

      if (
        diferencaX < 0
      )
      {

        proximaTela();
      }

      else
      {

        telaAnterior();
      }
    }

    else if (
      duracao < 1000
    )
    {

      proximaTela();
    }
  }
}


// ======================================================
// INSTAGRAM
// ======================================================

bool buscarSeguidores()
{

  if (
    WiFi.status() !=
    WL_CONNECTED
  )
  {

    return false;
  }


  if (
    strlen(INSTAGRAM_TOKEN) <
    20
  )
  {

    Serial.println(
      "Configure INSTAGRAM_TOKEN."
    );

    return false;
  }


  String url =
    "https://graph.instagram.com/";

  url +=
    INSTAGRAM_USER_ID;

  url +=
    "?fields=username,followers_count&access_token=";

  url +=
    INSTAGRAM_TOKEN;


  WiFiClientSecure client;

  client.setInsecure();


  HTTPClient http;


  if (
    !http.begin(
      client,
      url
    )
  )
  {

    return false;
  }


  http.setTimeout(
    20000
  );


  int codigo =
    http.GET();


  String resposta =
    http.getString();


  http.end();


  Serial.print(
    "Instagram HTTP: "
  );

  Serial.println(
    codigo
  );


  if (
    codigo != 200
  )
  {

    return false;
  }


  JsonDocument doc;


  if (
    deserializeJson(
      doc,
      resposta
    )
  )
  {

    return false;
  }


  long novoValor =
    doc["followers_count"] |
    0;


  if (
    novoValor <= 0
  )
  {

    return false;
  }


  seguidoresAnterior =
    seguidoresAtual;

  seguidoresAtual =
    novoValor;


  ultimaAtualizacaoInstagram =
    obterDataHora();


  primeiraLeitura =
    false;


  if (
    telaAtual ==
    TELA_INSTAGRAM
  )
  {

    desenharTelaInstagram();
  }


  return true;
}


// ======================================================
// BITCOIN
// ======================================================

bool buscarBitcoin()
{

  if (
    WiFi.status() !=
    WL_CONNECTED
  )
  {

    return false;
  }


  String url =
    "https://api.coingecko.com/api/v3/"
    "simple/price"
    "?ids=bitcoin"
    "&vs_currencies=usd,brl";


  WiFiClientSecure client;

  client.setInsecure();


  HTTPClient http;


  if (
    !http.begin(
      client,
      url
    )
  )
  {

    return false;
  }


  int codigo =
    http.GET();


  String resposta =
    http.getString();


  http.end();


  Serial.print(
    "Bitcoin HTTP: "
  );

  Serial.println(
    codigo
  );


  if (
    codigo != 200
  )
  {

    return false;
  }


  JsonDocument doc;


  if (
    deserializeJson(
      doc,
      resposta
    )
  )
  {

    return false;
  }


  bitcoinUSD =
    doc["bitcoin"]["usd"] |
    0.0;


  bitcoinBRL =
    doc["bitcoin"]["brl"] |
    0.0;


  ultimaAtualizacaoBitcoin =
    obterDataHora();


  if (
    telaAtual ==
    TELA_BITCOIN
  )
  {

    desenharTelaBitcoin();
  }


  return true;
}


// ======================================================
// CLIMA - OPEN METEO
// ======================================================

bool buscarClima()
{

  if (
    WiFi.status() !=
    WL_CONNECTED
  )
  {

    return false;
  }


  String url =
    "https://api.open-meteo.com/v1/forecast?latitude=";

  url +=
    String(
      LATITUDE,
      6
    );

  url +=
    "&longitude=";

  url +=
    String(
      LONGITUDE,
      6
    );

  url +=
    "&current=temperature_2m,"
    "relative_humidity_2m,"
    "apparent_temperature,"
    "precipitation,"
    "weather_code";

  url +=
    "&hourly=precipitation_probability";

  url +=
    "&forecast_days=1";

  url +=
    "&timezone=America%2FSao_Paulo";


  WiFiClientSecure client;

  client.setInsecure();


  HTTPClient http;


  if (
    !http.begin(
      client,
      url
    )
  )
  {

    return false;
  }


  http.setTimeout(
    15000
  );


  int codigo =
    http.GET();


  String resposta =
    http.getString();


  http.end();


  Serial.print(
    "Clima HTTP: "
  );

  Serial.println(
    codigo
  );


  if (
    codigo != 200
  )
  {

    Serial.println(
      resposta
    );

    return false;
  }


  JsonDocument doc;


  if (
    deserializeJson(
      doc,
      resposta
    )
  )
  {

    return false;
  }


  temperatura =
    doc["current"]["temperature_2m"] |
    0.0;


  sensacaoTermica =
    doc["current"]["apparent_temperature"] |
    0.0;


  umidade =
    doc["current"]["relative_humidity_2m"] |
    0;


  chuvaAgora =
    doc["current"]["precipitation"] |
    0.0;


  codigoClima =
    doc["current"]["weather_code"] |
    -1;


  // ==================================================
  // PROBABILIDADE DE CHUVA DA HORA ATUAL
  // ==================================================

  probabilidadeChuva =
    0;


  String horaAtual =
    doc["current"]["time"] |
    "";


  JsonArray tempos =
    doc["hourly"]["time"]
      .as<JsonArray>();


  JsonArray probabilidades =
    doc["hourly"]["precipitation_probability"]
      .as<JsonArray>();


  for (
    int i = 0;
    i < tempos.size();
    i++
  )
  {

    String hora =
      tempos[i]
        .as<String>();


    if (
      hora == horaAtual
    )
    {

      probabilidadeChuva =
        probabilidades[i] |
        0;

      break;
    }
  }


  ultimaAtualizacaoClima =
    obterDataHora();


  Serial.print(
    "Temperatura: "
  );

  Serial.println(
    temperatura
  );


  Serial.print(
    "Umidade: "
  );

  Serial.println(
    umidade
  );


  Serial.print(
    "Probabilidade chuva: "
  );

  Serial.println(
    probabilidadeChuva
  );


  if (
    telaAtual ==
    TELA_CLIMA
  )
  {

    desenharTelaClima();
  }


  return true;
}


// ======================================================
// SOL - SUNRISE-SUNSET.ORG V2
// ======================================================

bool buscarSol()
{

  if (
    WiFi.status() !=
    WL_CONNECTED
  )
  {

    return false;
  }


  String url =
    "https://api.sunrise-sunset.org/v2?lat=";

  url +=
    String(
      LATITUDE,
      6
    );

  url +=
    "&lng=";

  url +=
    String(
      LONGITUDE,
      6
    );

  url +=
    "&date=today";

  url +=
    "&tz=America%2FSao_Paulo";


  WiFiClientSecure client;

  client.setInsecure();


  HTTPClient http;


  if (
    !http.begin(
      client,
      url
    )
  )
  {

    return false;
  }


  http.setTimeout(
    15000
  );


  int codigo =
    http.GET();


  String resposta =
    http.getString();


  http.end();


  Serial.print(
    "Sol HTTP: "
  );

  Serial.println(
    codigo
  );


  if (
    codigo != 200
  )
  {

    Serial.println(
      resposta
    );

    return false;
  }


  JsonDocument doc;


  if (
    deserializeJson(
      doc,
      resposta
    )
  )
  {

    return false;
  }


  String nascer =
    doc["sunrise"] |
    "";


  String por =
    doc["sunset"] |
    "";


  long segundosDia =
    doc["day_length"] |
    0;


  horarioNascerSol =
    extrairHoraISO(
      nascer
    );


  horarioPorSol =
    extrairHoraISO(
      por
    );


  duracaoDia =
    formatarDuracaoDia(
      segundosDia
    );


  ultimaAtualizacaoSol =
    obterDataHora();


  Serial.print(
    "Nascer: "
  );

  Serial.println(
    horarioNascerSol
  );


  Serial.print(
    "Por do sol: "
  );

  Serial.println(
    horarioPorSol
  );


  if (
    telaAtual ==
    TELA_SOL
  )
  {

    desenharTelaSol();
  }


  return true;
}


// ======================================================
// COTACOES - FRANKFURTER V2
//
// Buscamos:
// USD -> BRL
// EUR -> BRL
// ======================================================

bool buscarCotacoes()
{

  if (
    WiFi.status() !=
    WL_CONNECTED
  )
  {

    return false;
  }


  // ==================================================
  // DOLAR
  // ==================================================

  {

    String url =
      "https://api.frankfurter.dev/v2/rate/usd/brl";


    WiFiClientSecure client;

    client.setInsecure();


    HTTPClient http;


    if (
      http.begin(
        client,
        url
      )
    )
    {

      int codigo =
        http.GET();


      String resposta =
        http.getString();


      http.end();


      Serial.print(
        "USD/BRL HTTP: "
      );

      Serial.println(
        codigo
      );


      if (
        codigo == 200
      )
      {

        JsonDocument doc;


        if (
          !deserializeJson(
            doc,
            resposta
          )
        )
        {

          dolarBRL =
            doc["rate"] |
            0.0;


          String data =
            doc["date"] |
            "";


          if (
            data.length() >= 10
          )
          {

            dataCotacao =
              data.substring(
                8,
                10
              ) +
              "/" +
              data.substring(
                5,
                7
              );
          }
        }
      }
    }
  }


  // Pequena pausa entre APIs

  delay(
    200
  );


  // ==================================================
  // EURO
  // ==================================================

  {

    String url =
      "https://api.frankfurter.dev/v2/rate/eur/brl";


    WiFiClientSecure client;

    client.setInsecure();


    HTTPClient http;


    if (
      http.begin(
        client,
        url
      )
    )
    {

      int codigo =
        http.GET();


      String resposta =
        http.getString();


      http.end();


      Serial.print(
        "EUR/BRL HTTP: "
      );

      Serial.println(
        codigo
      );


      if (
        codigo == 200
      )
      {

        JsonDocument doc;


        if (
          !deserializeJson(
            doc,
            resposta
          )
        )
        {

          euroBRL =
            doc["rate"] |
            0.0;
        }
      }
    }
  }


  ultimaAtualizacaoCotacao =
    obterDataHora();


  Serial.print(
    "Dolar: R$ "
  );

  Serial.println(
    dolarBRL,
    4
  );


  Serial.print(
    "Euro: R$ "
  );

  Serial.println(
    euroBRL,
    4
  );


  if (
    telaAtual ==
    TELA_COTACOES
  )
  {

    desenharTelaCotacoes();
  }


  return
    dolarBRL > 0 &&
    euroBRL > 0;
}


// ======================================================
// WIFI
// ======================================================

void conectarWiFi()
{

  lcd.fillScreen(
    lcd.color565(
      0,
      70,
      145
    )
  );


  lcd.setTextDatum(
    middle_center
  );


  lcd.setFont(
    &fonts::Font2
  );

  lcd.setTextColor(
    TFT_WHITE
  );


  lcd.drawString(
    "Conectando Wi-Fi...",
    160,
    105
  );


  lcd.setFont(
    &fonts::Font0
  );


  lcd.drawString(
    "LC SISTEMAS",
    160,
    140
  );


  WiFi.mode(
    WIFI_STA
  );


  WiFiManager wm;


  wm.setConfigPortalTimeout(
    180
  );


  bool conectado =
    wm.autoConnect(
      "LC SMART DISPLAY"
    );


  if (
    !conectado
  )
  {

    lcd.fillScreen(
      TFT_RED
    );


    lcd.setTextColor(
      TFT_WHITE
    );


    lcd.drawString(
      "Falha no Wi-Fi",
      160,
      120
    );


    delay(
      3000
    );


    ESP.restart();
  }


  Serial.println(
    "WiFi conectado."
  );


  Serial.print(
    "IP: "
  );

  Serial.println(
    WiFi.localIP()
  );
}


// ======================================================
// SETUP
// ======================================================

void setup()
{

  Serial.begin(
    115200
  );


  delay(
    500
  );


  Serial.println();
  Serial.println(
    "======================================"
  );

  Serial.println(
    "LC SISTEMAS SMART DISPLAY"
  );

  Serial.println(
    "8 TELAS"
  );

  Serial.println(
    "======================================"
  );


  // BACKLIGHT

  pinMode(
    TFT_BL,
    OUTPUT
  );


  digitalWrite(
    TFT_BL,
    HIGH
  );


  // DISPLAY

  lcd.init();

  lcd.setRotation(
    1
  );

  lcd.setBrightness(
    255
  );


  // TESTE

  lcd.fillScreen(
    TFT_RED
  );

  delay(120);

  lcd.fillScreen(
    TFT_GREEN
  );

  delay(120);

  lcd.fillScreen(
    TFT_BLUE
  );

  delay(120);

  lcd.fillScreen(
    TFT_BLACK
  );


  // ==================================================
  // CORES
  // ==================================================

  COR_CARD =
    lcd.color565(
      0,
      82,
      170
    );


  COR_CARD_BORDA =
    lcd.color565(
      80,
      175,
      255
    );


  COR_AZUL_CLARO =
    lcd.color565(
      185,
      225,
      255
    );


  COR_AZUL_RELOGIO =
    lcd.color565(
      0,
      90,
      200
    );


  COR_BITCOIN =
    lcd.color565(
      247,
      147,
      26
    );


  COR_ONLINE =
    lcd.color565(
      20,
      190,
      80
    );


  COR_OFFLINE =
    lcd.color565(
      230,
      55,
      55
    );


  COR_TEXTO_IOT =
    lcd.color565(
      35,
      45,
      60
    );


  COR_CINZA_IOT =
    lcd.color565(
      110,
      120,
      135
    );


  COR_CLIMA =
    lcd.color565(
      0,
      105,
      190
    );


  COR_SOL =
    lcd.color565(
      230,
      130,
      0
    );


  COR_COTACAO =
    lcd.color565(
      20,
      130,
      70
    );


  COR_COMENTARIOS =
    lcd.color565(
      120,
      45,
      160
    );


  // WIFI

  conectarWiFi();


  // ==================================================
  // NTP UTC-3
  // ==================================================

  configTime(
    -3 * 3600,
    0,
    "pool.ntp.org",
    "time.google.com",
    "time.cloudflare.com"
  );


  struct tm timeinfo;


  int tentativas =
    0;


  while (
    !getLocalTime(
      &timeinfo,
      1000
    ) &&
    tentativas < 10
  )
  {

    Serial.println(
      "Aguardando NTP..."
    );

    tentativas++;
  }


  // ==================================================
  // PRIMEIRA TELA
  // ==================================================

  atualizarDispositivosIoT();


  telaAtual =
    TELA_INSTAGRAM;


  desenharTelaAtual();


  // ==================================================
  // PRIMEIRAS CONSULTAS
  // ==================================================

  buscarSeguidores();

  buscarBitcoin();

  buscarClima();

  buscarSol();

  buscarCotacoes();

  buscarComentariosRecentes();


  // ==================================================
  // TIMERS
  // ==================================================

  unsigned long agora =
    millis();


  ultimaConsultaInstagram =
    agora;

  ultimaConsultaBitcoin =
    agora;

  ultimaConsultaClima =
    agora;

  ultimaConsultaSol =
    agora;

  ultimaConsultaCotacoes =
    agora;

  ultimaConsultaComentarios =
    agora;

  ultimaTrocaTela =
    agora;

  ultimaAtualizacaoRelogio =
    agora;
}


// ======================================================
// LOOP
// ======================================================

void loop()
{

  // ==================================================
  // WIFI
  // ==================================================

  if (
    WiFi.status() !=
    WL_CONNECTED
  )
  {

    static unsigned long ultimaTentativa =
      0;


    if (
      millis() -
      ultimaTentativa >
      10000
    )
    {

      ultimaTentativa =
        millis();


      Serial.println(
        "Reconectando WiFi..."
      );


      WiFi.reconnect();
    }
  }


  // ==================================================
  // TOUCH
  // ==================================================

  verificarTouch();


  // ==================================================
  // CARROSSEL
  // ==================================================

  if (
    millis() -
    ultimaTrocaTela >=
    INTERVALO_TELA
  )
  {

    proximaTela();
  }


  // ==================================================
  // RELOGIO
  // ==================================================

  if (
    telaAtual ==
      TELA_RELOGIO &&
    millis() -
      ultimaAtualizacaoRelogio >=
      1000
  )
  {

    ultimaAtualizacaoRelogio =
      millis();


    desenharTelaRelogio();
  }


  // ==================================================
  // INSTAGRAM
  // ==================================================

  if (
    millis() -
    ultimaConsultaInstagram >=
    INTERVALO_INSTAGRAM
  )
  {

    ultimaConsultaInstagram =
      millis();


    buscarSeguidores();
  }


  // ==================================================
  // BITCOIN
  // ==================================================

  if (
    millis() -
    ultimaConsultaBitcoin >=
    INTERVALO_BITCOIN
  )
  {

    ultimaConsultaBitcoin =
      millis();


    buscarBitcoin();
  }


  // ==================================================
  // CLIMA
  // ==================================================

  if (
    millis() -
    ultimaConsultaClima >=
    INTERVALO_CLIMA
  )
  {

    ultimaConsultaClima =
      millis();


    buscarClima();
  }


  // ==================================================
  // SOL
  // ==================================================

  if (
    millis() -
    ultimaConsultaSol >=
    INTERVALO_SOL
  )
  {

    ultimaConsultaSol =
      millis();


    buscarSol();
  }


  // ==================================================
  // COTACOES
  // ==================================================

  if (
    millis() -
    ultimaConsultaCotacoes >=
    INTERVALO_COTACOES
  )
  {

    ultimaConsultaCotacoes =
      millis();


    buscarCotacoes();
  }


  // ==================================================
  // COMENTARIOS INSTAGRAM - A CADA 5 MINUTOS
  // ==================================================

  if (
    millis() -
    ultimaConsultaComentarios >=
    INTERVALO_COMENTARIOS
  )
  {

    ultimaConsultaComentarios =
      millis();


    buscarComentariosRecentes();
  }


  delay(
    20
  );
}
