#!/usr/bin/perl

BEGIN {
   push @INC, '/usr/share/request-tracker4/lib';
}

use strict;
use warnings;
use RT::Interface::CLI;
use RT;

require RT::Tickets;

RT::LoadConfig();
RT::Init();

my $currentUser = RT::Interface::CLI::GetCurrentUser();

my $tickets = RT::Tickets->new( $currentUser );

$tickets->LimitQueue( VALUE => 'GestaoDeVulnerabilidades');
$tickets->LimitStatus( VALUE => 'open');
$tickets->LimitLastUpdated(OPERATOR => '<', VALUE => '7 days ago'); 
#$ticket->FirstCustomFieldValue retorna o status igual o da string passada
while ( my $ticket = $tickets->Next ) {
  if ( $ticket->FirstCustomFieldValue('NotificationStatus') eq 'Cliente notificado' ) { # Se o cliente já tiver sido notificado, envia uma nova mensagem.   
     #Cliente Notificado: Primeiro status depois de um tratamento. Usado para notificar de que o cliente recebeu a notificação.
     #Equipe Notificada: Usado para indicar que a equipe interna foi notificada sobre algo em relação ao ticket. 
     #AguardandoResposta: Usado para indicar que o cliente já foi notificado uma vez, e a equipe interna está esperando uma notificação do cliente.
     my $emails = $ticket->CcAddresses;
     #$emails = substr($emails, 0, -2);
     #my $message << "end_of_file"; Exemplo de Here-Document. Única maneira do Mime aceitar uma mensagem sem criar um documento novo. 
     my $message = <<'END_MESSAGE';
Prezado(a),

Identificamos que a notificação de vulnerabilidade de segurança ainda 
não foi atualizada com o tratamento realizado pela sua instituição. 
Salientamos a importância de realizar o tratamento das vulnerabilidades
de segurança reportados a sua organização, contemplando desde a mitigação
até a resposta ao e-mail de notificação.

Ressaltamos que estamos à disposição para ajudá-los como possível no
tratamento dessa demanda. Caso tenha alguma dúvida ou dificuldade técnica,
não hesite em nos contactar.

Caso não seja o contato responsável por receber a notificação de
vulnerabilidades de segurança, pedimos a gentileza de encaminhar esta
mensagem ao setor responsável e nos informar para atualizarmos nossos
registros.

No aguardo de um retorno, agradecemos pela atenção.

Atenciosamente,

--
Grupo de Resposta a Incidentes de Segurança da Bahia/Brasil
http://certbahia.pop-ba.rnp.br
Tel.: +55 71 3283-6098 / 3283-5693
END_MESSAGE

     # Cria a mensagem e envia 
     my $mime = MIME::Entity->build(
        From => 'CERT.Bahia <incidentes@pop-ba.rnp.br>',
        Data => $message
     );
     $ticket->Correspond(
        MIMEObj => $mime,
        CcMessageTo => $emails
     );

     $ticket->AddCustomFieldValue(
        Field => 'NotificationStatus',
        Value => 'AguardandoResposta'
     );
  } elsif ( $ticket->FirstCustomFieldValue('NotificationStatus') eq 'AguardandoResposta' ) {
     my $message = <<'END_MESSAGE';
Atenção!!

Esse ticket já está há 7 dias sem atualização.

Favor ligar e verificar se o cliente necessita de auxílio no tratamento da vulnerabilidade.
END_MESSAGE

     my $mime = MIME::Entity->build(
        From => 'CERT.Bahia <incidentes@pop-ba.rnp.br>',
        Data => $message
     );

     # Notifica a equipe internamente com um cometário 
     $ticket->Comment(
        MIMEObj => $mime
     );

     $ticket->AddCustomFieldValue(
        Field => 'NotificationStatus',
        Value => 'Equipe notificada'
     );
} elsif ( ($ticket->FirstCustomFieldValue('NotificationStatus') eq 'Equipe notificada') and ($ticket->StartedObj->Diff <=  -2628000)) { #2628000 é um mês em segundos
      my $Diff = $ticket->StartedObj->Diff; #Diff retorna o valor escalar da diferença entre o tempo da abertura do objeto e a data atual  
	  	my $message = << "END_MESSAGE"; 
Atenção!!
Ainda sem atualização, fechando o ticket;
END_MESSAGE
     	my $mime = MIME::Entity->build(
      	  From => 'CERT.Bahia <incidentes@pop-ba.rnp.br>',
      	  Data => $message
     	);

     	# Notifica a equipe internamente com um comentário 
    	 $ticket->Comment(
      	  MIMEObj => $mime
    	 );

    	 $ticket->AddCustomFieldValue(
    	   Field => 'NotificationStatus',
  	      Value => 'Fechado por falta de resposta do cliente'
		);
		$ticket->SetStatus('resolved');
  } 
}
