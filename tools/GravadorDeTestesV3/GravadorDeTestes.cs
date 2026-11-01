using System;
using System.IO;
using System.IO.Ports;
using System.Text;
using System.Text.RegularExpressions;
using System.Globalization;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Windows.Forms;
using System.Windows.Forms.DataVisualization.Charting;

static class Dados
{
    public static readonly string[] Campos = {"Alvo", "Posicao", "P", "I", "D", "FF", "PWM", "Falha", "ACS_mV", "Corrente_mA", "ACS_OK", "Amp_OK", "POS", "POT", "VEL", "RPM", "PICO_RPM", "ESTADO", "ACS_ADC", "ESCALA", "ZERO", "Temp_C", "NTC_OK", "NTC_ADC", "NTC_estado", "Tempo_ms", "Modo", "B10K_ADC"};
    static readonly string[] CamposD14={"Tempo_ms","Modo","POS","B10K_ADC","Posicao","Alvo","P","I","D","FF","VEL","RPM","PICO_RPM","PWM","ESTADO","Falha","ACS_ADC","ACS_mV","Corrente_mA","ACS_OK","Amp_OK","ESCALA","ZERO","Temp_C","NTC_OK","NTC_ADC","NTC_estado"};
    public static string VersaoMonitor(string linha) {
        if(linha.StartsWith("V15 POS=",StringComparison.Ordinal)) return "V15";
        if(linha.StartsWith("V14 POS=",StringComparison.Ordinal)) return "V14";
        if(linha.StartsWith("V13 POS=",StringComparison.Ordinal)) return "V13";
        return "";
    }
    static readonly Regex Par = new Regex(@"([A-Za-z_]+)[:=]([-+]?(?:\d+(?:\.\d*)?|\.\d+))", RegexOptions.Compiled);
    public static Dictionary<string, string> Ler(string linha)
    {
        if(linha.StartsWith("D14;",StringComparison.Ordinal) || linha.StartsWith("D15;",StringComparison.Ordinal)) {
            string[] partes=linha.Split(';');
            if(partes.Length!=CamposD14.Length+1) return null;
            var quadro=new Dictionary<string,string>(StringComparer.Ordinal);
            for(int i=0;i<CamposD14.Length;i++) {
                double valor;
                if(!double.TryParse(partes[i+1],NumberStyles.Float,CultureInfo.InvariantCulture,out valor) || double.IsNaN(valor) || double.IsInfinity(valor)) return null;
                quadro[CamposD14[i]]=partes[i+1];
            }
            return quadro;
        }
        bool grafico = linha.StartsWith("Alvo:", StringComparison.Ordinal);
        bool monitor = VersaoMonitor(linha)!="";
        if (!grafico && !monitor) return null;
        var resultado = new Dictionary<string, string>(StringComparer.Ordinal);
        foreach (Match m in Par.Matches(linha)) resultado[m.Groups[1].Value] = m.Groups[2].Value;
        if (monitor)
        {
            Copiar(resultado, "ALVO", "Alvo"); Copiar(resultado, "ANG", "Posicao");
            Copiar(resultado, "FALHA", "Falha"); Copiar(resultado, "mA", "Corrente_mA");
        }
        foreach (string chave in new string[]{"Alvo", "Posicao", "PWM", "Falha"})
            if (!resultado.ContainsKey(chave)) return null;
        if (grafico)
            foreach (string chave in new string[]{"P", "I", "D", "FF", "ACS_mV", "Corrente_mA", "ACS_OK", "Amp_OK"})
                if (!resultado.ContainsKey(chave)) return null;
        return resultado;
    }
    static void Copiar(Dictionary<string, string> d, string de, string para) { if (d.ContainsKey(de)) d[para] = d[de]; }
    public static double Numero(Dictionary<string, string> d, string chave)
    {
        return d.ContainsKey(chave) ? double.Parse(d[chave], CultureInfo.InvariantCulture) : double.NaN;
    }
    public static string ValorCSV(string valor)
    {
        return double.Parse(valor, CultureInfo.InvariantCulture).ToString("0.########", CultureInfo.GetCultureInfo("pt-BR"));
    }
}

sealed class Registro : IDisposable
{
    public string Pasta;
    public long Quantidade;
    public bool Falhou;
    public readonly Stopwatch Relogio = Stopwatch.StartNew();
    StreamWriter csv, bruto;
    string titulo, versao, porta;
    double erroMax, somaErro, correnteMax, pwmMax, temperaturaMax=double.MinValue;
    long amostrasCorrente, amostrasTemperatura;
    public Registro(string raiz, string nome, string firmware, string com)
    {
        titulo = nome; versao = firmware; porta = com;
        string baseNome = DateTime.Now.ToString("yyyy-MM-dd_HH-mm-ss-fff") + "_" + nome + "_" + firmware;
        Pasta = Path.Combine(raiz, baseNome);
        for (int i = 2; Directory.Exists(Pasta); i++) Pasta = Path.Combine(raiz, baseNome + "_" + i);
        Directory.CreateDirectory(Pasta);
        csv = new StreamWriter(Path.Combine(Pasta, "dados.csv"), false, new UTF8Encoding(true));
        bruto = new StreamWriter(Path.Combine(Pasta, "serial.txt"), false, new UTF8Encoding(true));
        csv.AutoFlush = bruto.AutoFlush = true;
        csv.WriteLine("Horario_PC;Tempo_s;Formato;" + string.Join(";", Dados.Campos));
        Descrever("Em andamento");
    }
    void Descrever(string fim)
    {
        File.WriteAllText(Path.Combine(Pasta, "teste.txt"),
            "Teste: " + titulo + "\r\nFirmware detectado: " + versao + "\r\nPorta: " + porta +
            "\r\nLinhas de dados: " + Quantidade + "\r\nFalha observada: " + Falhou + "\r\nSituacao: " + fim +
            "\r\nDuracao de recebimento (s): " + Relogio.Elapsed.TotalSeconds.ToString("F2",CultureInfo.InvariantCulture) +
            "\r\nMaior erro absoluto de posicao (graus nominais): " + erroMax.ToString("F2",CultureInfo.InvariantCulture) +
            "\r\nErro absoluto medio (graus nominais): " + (Quantidade>0?somaErro/Quantidade:0).ToString("F2",CultureInfo.InvariantCulture) +
            "\r\nMaior modulo de PWM observado: " + pwmMax.ToString("F0",CultureInfo.InvariantCulture) +
            "\r\nMaior modulo de corrente observado (mA): " + (amostrasCorrente>0?correnteMax.ToString("F1",CultureInfo.InvariantCulture):"Indisponivel") +
            "\r\nMaior temperatura observada (C): " + (amostrasTemperatura>0?temperaturaMax.ToString("F1",CultureInfo.InvariantCulture):"Indisponivel") +
            "\r\nTempo e horario sao do recebimento no PC. CSV: ponto e virgula, decimal virgula." +
            "\r\nCampo vazio = nao transmitido nesse formato, e nao valor zero." +
            "\r\nCorrente_mA so e valida quando Amp_OK=1; ACS_mV quando ACS_OK=1." +
            "\r\nTemp_C so e valida quando NTC_OK=1. NTC configurado: 100k/B3950, resistor fixo 4k7; sensor fisico ainda precisa ser conferido." +
            "\r\nO nome identifica o modo pedido; nao comprova movimento ou sucesso fisico.\r\n", new UTF8Encoding(true));
    }
    public void Evento(string mensagem) { bruto.WriteLine(DateTimeOffset.Now.ToString("o") + " " + mensagem); }
    public Dictionary<string, string> Receber(string linha)
    {
        Evento("RX " + linha);
        var d = Dados.Ler(linha);
        if (d == null) return null;
        var valores = new List<string>();
        valores.Add(DateTimeOffset.Now.ToString("o"));
        valores.Add(Relogio.Elapsed.TotalSeconds.ToString("0.000", CultureInfo.GetCultureInfo("pt-BR")));
        valores.Add(linha.StartsWith("D15;")?"ColetaV15":linha.StartsWith("D14;")?"ColetaV14":Dados.VersaoMonitor(linha)!="" ? "Monitor" : "Plotter");
        foreach (string campo in Dados.Campos) valores.Add(d.ContainsKey(campo) ? Dados.ValorCSV(d[campo]) : "");
        csv.WriteLine(string.Join(";", valores.ToArray()));
        Quantidade++; if (Dados.Numero(d, "Falha") != 0) Falhou = true;
        double erro=Math.Abs(Dados.Numero(d,"Alvo")-Dados.Numero(d,"Posicao"));
        somaErro+=erro; erroMax=Math.Max(erroMax,erro); pwmMax=Math.Max(pwmMax,Math.Abs(Dados.Numero(d,"PWM")));
        if(Dados.Numero(d,"Amp_OK")==1) { correnteMax=Math.Max(correnteMax,Math.Abs(Dados.Numero(d,"Corrente_mA"))); amostrasCorrente++; }
        if(Dados.Numero(d,"NTC_OK")==1) { temperaturaMax=Math.Max(temperaturaMax,Dados.Numero(d,"Temp_C")); amostrasTemperatura++; }
        return d;
    }
    public void Encerrar(string motivo)
    {
        if (csv == null) return;
        Evento("FIM " + motivo); csv.Dispose(); bruto.Dispose(); csv = bruto = null;
        Descrever(motivo);
    }
    public void Dispose() { Encerrar("Encerrado"); }
}

sealed class Janela : Form
{
    readonly string raiz = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments), "ServoInteligente", "Testes");
    readonly string preferencia = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "ultima_porta.txt");
    ComboBox portas = new ComboBox(), sensor = new ComboBox();
    Button conectar = new Button();
    Label estado = new Label(), leitura = new Label(), arquivo = new Label();
    Chart grafico = new Chart();
    TextBox mensagens = new TextBox();
    List<Button> movimentos = new List<Button>();
    System.Windows.Forms.Timer timer = new System.Windows.Forms.Timer();
    SerialPort serial;
    Registro registro;
    string buffer = "", versao = "Aguardando_placa";
    bool pronto = false, fechando = false;
    long desligarEm = -1;
    Stopwatch espera = new Stopwatch();

    public Janela(bool previa)
    {
        Text = "Gravador de testes do servo - V3"; ClientSize = new Size(950, 650); MinimumSize = new Size(900, 640);
        Font = new Font("Segoe UI", 10); StartPosition = FormStartPosition.CenterScreen;
        var topo = new FlowLayoutPanel { Dock = DockStyle.Top, Height = 45, Padding = new Padding(8) };
        topo.Controls.Add(new Label {Text="Placa:", AutoSize=true, Padding=new Padding(0,5,0,0)});
        portas.Width=115; portas.DropDownStyle=ComboBoxStyle.DropDownList; topo.Controls.Add(portas);
        var atualizar = Botao("Atualizar", delegate { AtualizarPortas(null); }); topo.Controls.Add(atualizar);
        conectar = Botao("Conectar", delegate { if (serial == null) Conectar(); else Desconectar("Desconectado"); }); topo.Controls.Add(conectar);
        topo.Controls.Add(new Label {Text="ACS:",AutoSize=true,Padding=new Padding(15,5,0,0)});
        sensor.Width=160; sensor.DropDownStyle=ComboBoxStyle.DropDownList;
        sensor.Items.AddRange(new object[]{"Nao identificado", "5 A / 05B", "20 A", "30 A"}); sensor.SelectedIndex=0;
        sensor.SelectedIndexChanged += delegate { if(pronto) Enviar(sensor.SelectedIndex.ToString()); };
        topo.Controls.Add(sensor);
        var abrir=Botao("Ver testes salvos", delegate { Directory.CreateDirectory(raiz); Process.Start("explorer.exe", "\""+raiz+"\""); }); topo.Controls.Add(abrir);
        var comandos = new FlowLayoutPanel {Dock=DockStyle.Top, Height=50, Padding=new Padding(8)};
        AdicionarModo(comandos,"Suave", "p", "B10K_suave_PID");
        AdicionarModo(comandos,"Rapido", "f", "B10K_rapido_PID");
        AdicionarModo(comandos,"Vai e vem", "i", "Vaivem_TT_PID");
        AdicionarModo(comandos,"Dois servos", "t", "Vaivem_dois_servos");
        AdicionarModo(comandos,"Servo normal", "s", "Servo_normal_B10K");
        AdicionarModo(comandos,"Zerar corrente", "z", "Zero_ACS712");
        var parar=Botao("PARAR", delegate { if(serial!=null) { Enviar("x"); if(registro!=null) desligarEm=registro.Relogio.ElapsedMilliseconds+1000; estado.Text="Parado. Finalizando o registro automaticamente."; } });
        parar.BackColor=Color.MistyRose; comandos.Controls.Add(parar);
        var rodape = new Panel {Dock=DockStyle.Bottom,Height=155,Padding=new Padding(10)};
        estado.SetBounds(10,5,910,25); estado.Anchor=AnchorStyles.Left|AnchorStyles.Right|AnchorStyles.Top;
        leitura.SetBounds(10,33,910,25); leitura.Anchor=estado.Anchor;
        arquivo.SetBounds(10,60,910,25); arquivo.Anchor=estado.Anchor; arquivo.AutoEllipsis=true;
        mensagens.SetBounds(10,88,910,60); mensagens.Anchor=estado.Anchor|AnchorStyles.Bottom;
        mensagens.Multiline=true; mensagens.ReadOnly=true; mensagens.ScrollBars=ScrollBars.Vertical;
        rodape.Controls.AddRange(new Control[]{estado,leitura,arquivo,mensagens});
        grafico.Dock=DockStyle.Fill; var area=new ChartArea("Posicao");
        area.AxisX.Title="Tempo do teste (s)"; area.AxisY.Title="Graus nominais";
        area.AxisY2.Title="Temperatura (C)";
        grafico.ChartAreas.Add(area); grafico.Legends.Add(new Legend("Sinais"));
        foreach(string nome in new string[]{"Alvo","Posicao"}) { var s=new Series(nome); s.ChartType=SeriesChartType.Line; s.BorderWidth=2; grafico.Series.Add(s); }
        var curvaTemp=new Series("Temperatura"); curvaTemp.ChartType=SeriesChartType.Line; curvaTemp.YAxisType=AxisType.Secondary; curvaTemp.BorderWidth=2; grafico.Series.Add(curvaTemp);
        Controls.Add(grafico); Controls.Add(rodape); Controls.Add(comandos); Controls.Add(topo);
        estado.Text="Conecte a placa. Cada botao inicia e salva um teste com nome automatico.";
        arquivo.Text="Pasta fixa: "+raiz;
        if(!previa) Directory.CreateDirectory(raiz);
        string anterior=null; try { if(File.Exists(preferencia)) anterior=File.ReadAllText(preferencia).Trim(); } catch{}
        AtualizarPortas(anterior); Habilitar(false);
        timer.Interval=30; timer.Tick += delegate { Receber(); }; if(!previa) timer.Start();
        FormClosing += delegate { fechando=true; timer.Stop(); Desconectar("Janela fechada"); };
        Shown += delegate { if(!previa && anterior!=null && portas.SelectedItem!=null && portas.SelectedItem.ToString()==anterior) Conectar(); };
        if(previa)
        {
            estado.Text="PREVIA com dados ficticios - nenhuma placa conectada";
            leitura.Text="Pos: 108,0 graus   |   PWM: 90   |   Corrente: 250 mA   |   Temp: 32,0 C";
            for(int i=0;i<200;i++) { grafico.Series["Alvo"].Points.AddXY(i*.05,108+60*Math.Sin(i*.035)); grafico.Series["Posicao"].Points.AddXY(i*.05,108+58*Math.Sin(i*.035-.08)); grafico.Series["Temperatura"].Points.AddXY(i*.05,25+i*.035); }
        }
    }
    Button Botao(string titulo, EventHandler acao)
    {
        var b=new Button { Text=titulo, AutoSize=true, Height=30, Margin=new Padding(3) }; b.Click+=acao; return b;
    }
    void Habilitar(bool valor) { foreach(var b in movimentos) b.Enabled=valor; sensor.Enabled=valor; }
    void AdicionarModo(FlowLayoutPanel painel,string titulo,string comando,string nome)
    {
        var b=Botao(titulo,delegate { if(NovoTeste(nome)) Enviar((versao=="V14" || versao=="V15"?"c":"g")+comando); }); movimentos.Add(b); painel.Controls.Add(b);
    }
    void AtualizarPortas(string preferida)
    {
        if(serial!=null) return;
        string atual=preferida ?? (portas.SelectedItem==null?null:portas.SelectedItem.ToString());
        portas.Items.Clear(); var lista=SerialPort.GetPortNames(); Array.Sort(lista); portas.Items.AddRange(lista);
        if(atual!=null && portas.Items.Contains(atual)) portas.SelectedItem=atual;
        else if(portas.Items.Count>0) portas.SelectedIndex=0;
    }
    void Conectar()
    {
        if(portas.SelectedItem==null) { estado.Text="Nenhuma porta encontrada. Ligue o USB e clique Atualizar."; return; }
        try
        {
            serial=new SerialPort(portas.SelectedItem.ToString(),115200,Parity.None,8,StopBits.One);
            serial.DtrEnable=true; serial.ReadTimeout=100; serial.WriteTimeout=500; serial.ReadBufferSize=65536;
            serial.Open(); buffer=""; pronto=false; versao="Aguardando_placa";
            if(!NovoTeste("Observacao")) return;
            Enviar("xl"); espera.Restart();
            conectar.Text="Desconectar"; portas.Enabled=false;
            estado.Text="Conectado. Motor parado; identificando V13/V14/V15...";
        }
        catch(Exception ex) { Desconectar("Falha ao conectar: "+ex.Message); }
    }
    bool NovoTeste(string nome)
    {
        if(serial==null) return false;
        try
        {
            if(registro!=null) registro.Encerrar("Novo teste selecionado");
            registro=new Registro(raiz,nome,versao,serial.PortName); desligarEm=-1;
            arquivo.Text="Salvando: "+registro.Pasta;
            foreach(var s in grafico.Series) s.Points.Clear();
            estado.Text="Gravando automaticamente: "+nome;
            return true;
        }
        catch(Exception ex)
        {
            try { if(serial!=null && serial.IsOpen) serial.Write("x"); } catch{}
            registro=null; Desconectar("Nao foi possivel salvar. Parada solicitada. "+ex.Message);
            MessageBox.Show(estado.Text,"Gravacao interrompida"); return false;
        }
    }
    void Enviar(string texto)
    {
        if(serial==null || !serial.IsOpen) return;
        try { serial.Write(texto); if(registro!=null) registro.Evento("TX "+texto); }
        catch(Exception ex) { Desconectar("Falha no envio: "+ex.Message); }
    }
    void Receber()
    {
        if(serial==null) return;
        try
        {
            buffer+=serial.ReadExisting();
            int n;
            while((n=buffer.IndexOf('\n'))>=0)
            {
                string linha=buffer.Substring(0,n).TrimEnd('\r'); buffer=buffer.Substring(n+1);
                if(!pronto && Dados.VersaoMonitor(linha)!="" && Dados.Ler(linha)!=null)
                {
                    pronto=true; versao=Dados.VersaoMonitor(linha); Habilitar(true);
                    try { File.WriteAllText(preferencia,serial.PortName); } catch{}
                    if(!NovoTeste("Observacao")) return;
                    Enviar((versao=="V14" || versao=="V15"?"c":"g")+sensor.SelectedIndex.ToString());
                    estado.Text="Pronto. Escolha um teste; o nome e o salvamento sao automaticos.";
                }
                Dictionary<string,string> d=registro==null?Dados.Ler(linha):registro.Receber(linha);
                if(d==null) { if(linha.Length>0) Mensagem(linha); continue; }
                leitura.Text=string.Format(CultureInfo.InvariantCulture,"Pos: {0:F1} graus   |   PWM: {1:F0}   |   Corrente: {2}   |   Temp: {3}",
                    Dados.Numero(d,"Posicao"),Dados.Numero(d,"PWM"),Dados.Numero(d,"Amp_OK")==1 ? Dados.Numero(d,"Corrente_mA").ToString("F0")+" mA" : "indisponivel",
                    Dados.Numero(d,"NTC_OK")==1 ? Dados.Numero(d,"Temp_C").ToString("F1")+" C" : "indisponivel");
                if(registro!=null)
                {
                    foreach(var s in grafico.Series) {
                        bool ehTemp=s.Name=="Temperatura";
                        bool valido=!ehTemp || Dados.Numero(d,"NTC_OK")==1;
                        int ponto=s.Points.AddXY(registro.Relogio.Elapsed.TotalSeconds,valido?Dados.Numero(d,ehTemp?"Temp_C":s.Name):0);
                        if(!valido) s.Points[ponto].IsEmpty=true;
                        if(s.Points.Count>600) s.Points.RemoveAt(0);
                    }
                    if(Dados.Numero(d,"Falha")!=0) estado.Text="O servo registrou falha. O arquivo continua sendo salvo. Use PARAR para reconhecer.";
                }
            }
            if(buffer.Length>16384) throw new IOException("Recebimento sem fim de linha reconhecivel.");
            if(!pronto && espera.ElapsedMilliseconds>5000) estado.Text="V13/V14/V15 nao identificada. Confira a programacao e feche o Monitor/Plotter Arduino.";
            if(registro!=null && desligarEm>=0 && registro.Relogio.ElapsedMilliseconds>=desligarEm)
            {
                registro.Encerrar("Parada solicitada"); registro=null; desligarEm=-1;
                estado.Text="Teste salvo. O proximo botao cria outro registro automaticamente.";
            }
        }
        catch(Exception ex) { Desconectar("Registro interrompido: "+ex.Message); }
    }
    void Mensagem(string texto)
    {
        if(mensagens.TextLength>5000) mensagens.Text=mensagens.Text.Substring(mensagens.TextLength-3000);
        mensagens.AppendText(texto+Environment.NewLine);
    }
    void Desconectar(string motivo)
    {
        if(serial!=null)
        {
            try { if(serial.IsOpen) serial.Write("x"); } catch{}
            try { serial.Dispose(); } catch{} serial=null;
        }
        if(registro!=null) { try { registro.Encerrar(motivo); } catch{} registro=null; }
        pronto=false; Habilitar(false); portas.Enabled=true; conectar.Text="Conectar";
        estado.Text=motivo; if(!fechando) Mensagem(motivo);
    }
}

static class Programa
{
    [STAThread]
    static int Main(string[] args)
    {
        if(args.Length==2 && args[0]=="--testar") return Testar(args[1]);
        Application.EnableVisualStyles(); Application.SetCompatibleTextRenderingDefault(false);
        if(args.Length==2 && args[0]=="--prever")
        {
            using(var j=new Janela(true)) using(var b=new Bitmap(j.Width,j.Height)) { j.Opacity=0; j.Show(); Application.DoEvents(); j.DrawToBitmap(b,new Rectangle(0,0,b.Width,b.Height)); b.Save(args[1],System.Drawing.Imaging.ImageFormat.Png); j.Close(); }
            return 0;
        }
        Application.ThreadException += delegate(object remetente,System.Threading.ThreadExceptionEventArgs e) { MessageBox.Show(e.Exception.Message,"Gravador de testes"); };
        try { Application.Run(new Janela(false)); return 0; }
        catch(Exception ex) { MessageBox.Show(ex.Message,"Nao foi possivel abrir o gravador"); return 1; }
    }
    static int Testar(string raiz)
    {
        try
        {
            CultureInfo.CurrentCulture=CultureInfo.GetCultureInfo("pt-BR");
            string grafico="Alvo:108.2\tPosicao:107.3\tP:8.1\tI:-1.2\tD:0.3\tFF:2.0\tPWM:-90\tFalha:0\tACS_mV:18.5\tCorrente_mA:100.0\tACS_OK:1\tAmp_OK:1";
            string monitor="V13 POS=2000 POT=108.0 ANG=107.5 ALVO=108.2 VEL=-2.1 RPM=0.4 PICO_RPM=1.5 PWM=-90 ESTADO=3 FALHA=1 ACS_ADC=1551 ACS_mV=18.5 mA=100.0 ACS_OK=1 Amp_OK=1 ESCALA=1 ZERO=0";
            if(Dados.Ler("PAROU: Sem progresso.")!=null || Dados.Ler("Alvo:108 Posicao:107 PWM:0 Falha:0")!=null) throw new Exception("Aceitou quadro incompleto");
            var d=Dados.Ler(grafico); if(Dados.Numero(d,"I")!=-1.2) throw new Exception("Sinal/cultura");
            var r=new Registro(raiz,"B10K_suave_PID","V13","TESTE_SEM_PLACA");
            r.Receber(grafico); r.Receber(monitor); r.Receber("PAROU: Sem progresso."); r.Encerrar("Teste sintetico");
            string[] linhas=File.ReadAllLines(Path.Combine(r.Pasta,"dados.csv"));
            if(linhas.Length!=3 || !linhas[1].Contains("108,2") || !r.Falhou || r.Quantidade!=2) throw new Exception("CSV/resumo incorreto");
            if(linhas[0].Split(';').Length!=linhas[1].Split(';').Length || linhas[0].Split(';').Length!=linhas[2].Split(';').Length) throw new Exception("Colunas inconsistentes");
            var r2=new Registro(raiz,"B10K_suave_PID","V13","TESTE_SEM_PLACA"); r2.Encerrar("Teste sintetico");
            if(r.Pasta==r2.Pasta) throw new Exception("Sobrescreveu teste");
            string coleta="D14;12345;2;2000;2048;107.5;108.2;6.3;-1.2;0.3;2.0;-2.1;0.4;1.5;-90;3;0;1551;18.5;100.0;1;1;1;0;42.5;1;3500;1";
            var dados14=Dados.Ler(coleta);
            if(dados14==null || Dados.Numero(dados14,"Temp_C")!=42.5 || Dados.Numero(dados14,"VEL")!=-2.1 || Dados.Numero(dados14,"B10K_ADC")!=2048) throw new Exception("Campos V14 incorretos");
            if(Dados.Ler(coleta+";999")!=null || Dados.Ler(coleta.Replace("42.5","NaN"))!=null || Dados.Ler(coleta.Replace("42.5","Infinity"))!=null) throw new Exception("Aceitou quadro V14 invalido");
            if(Dados.VersaoMonitor(monitor.Replace("V13 ","V14 "))!="V14") throw new Exception("Identificacao V14");
            var r14=new Registro(raiz,"B10K_suave_PID","V14","TESTE_SEM_PLACA");
            r14.Receber(coleta); r14.Receber(coleta.Replace(";42.5;1;3500;1",";0;0;4095;3")); r14.Encerrar("Teste sintetico V14");
            string resumo14=File.ReadAllText(Path.Combine(r14.Pasta,"teste.txt"));
            if(!resumo14.Contains("42.5") || r14.Quantidade!=2) throw new Exception("Resumo de temperatura");
            var csv14=File.ReadAllLines(Path.Combine(r14.Pasta,"dados.csv"));
            if(csv14[0].Split(';').Length!=csv14[1].Split(';').Length || !csv14[1].Contains("42,5")) throw new Exception("CSV V14");
            var d15=Dados.Ler(coleta.Replace("D14;","D15;"));
            if(d15==null || Dados.Numero(d15,"D")!=.3 || Dados.VersaoMonitor(monitor.Replace("V13 ","V15 "))!="V15") throw new Exception("V15");
            var r15=new Registro(raiz,"B10K_rapido_PID","V15","TESTE_SEM_PLACA");
            r15.Receber(coleta.Replace("D14;","D15;")); r15.Encerrar("Teste sintetico V15");
            if(!File.ReadAllText(Path.Combine(r15.Pasta,"dados.csv")).Contains("ColetaV15")) throw new Exception("CSV V15");
            File.WriteAllText(Path.Combine(raiz,"resultado.txt"),"OK: V13/V14/V15, coleta completa, temperatura, sensor invalido, sinais, pt-BR, CSV, resumo e nomes unicos. Sem acesso a porta serial.");
            return 0;
        }
        catch(Exception ex) { Directory.CreateDirectory(raiz); File.WriteAllText(Path.Combine(raiz,"resultado.txt"),ex.ToString()); return 1; }
    }
}
