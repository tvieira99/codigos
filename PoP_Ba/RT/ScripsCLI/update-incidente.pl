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

$tickets->LimitQueue( VALUE => 'Incidentes_Seguranca');
$tickets->LimitStatus( VALUE => 'open');
$tickets->LimitLastUpdated(OPERATOR => '<', VALUE => '7 days ago');
#$ticket->FirstCustomFieldValue retorna o status igual o da string passada
while ( my $ticket = $tickets->Next ) {
  if ( $ticket->FirstCustomFieldValue('NotificationStatus') eq 'Cliente notificado' ) { 
     $ticket->AddCustomFieldValue(
        Field => 'NotificationStatus',
        Value => 'AguardandoResposta'
     );
      # Se o cliente já tiver sido notificado, envia uma nova mensagem.   
     #Cliente Notificado: Primeiro status depois de um tratamento. Usado para notificar de que o cliente recebeu a notificação.
     #Equipe Notificada: Usado para indicar que a equipe interna foi notificada sobre algo em relação ao ticket. 
     my $emails = $ticket->CcAddresses;
     #my $pop_regex = "\@pop-ba.rnp.br\$"
     #$emails = substr($emails, 0, -2);
     # my $message << "end_of_file"; Exemplo de Here-Document. Única maneira do Mime aceitar uma mensagem sem criar um documento novo. 
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

     # create the message and send
     my $mime = MIME::Entity->build(
        From => 'CERT.Bahia <incidentes@pop-ba.rnp.br>',
        Data => $message
     );
     $ticket->Correspond(
        MIMEObj => $mime,
        CcMessageTo => $emails
     );
     
  } elsif ( $ticket->FirstCustomFieldValue('NotificationStatus') eq 'AguardandoResposta') {
	  	my $message = <<'END_MESSAGE';
Atenção!!
Ainda sem atualização, fechando o ticket;
END_MESSAGE
     	my $mime = MIME::Entity->build(
      	  From => 'CERT.Bahia <incidentes@pop-ba.rnp.br>',
      	  Data => $message
     	);

     	# notify internal team about the ticket to call the entity
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
