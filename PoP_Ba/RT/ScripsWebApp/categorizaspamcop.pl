# set para 1 se quiset habilitar debug
my $debug = 0;

use LWP::UserAgent;    #NECESSÁRIO PRA FAZER O HTTP GET PRA API
use XML::Simple;       #NECESSÁRIO PRA PARSEAR O XML

my %ticketParams = (
    "notificationsource" => "",
    "incidentcategory"   => "",
    "instituicao"        => "",
    "camp"               => ""
);

my $ticket = $self->TicketObj;

$RT::Logger->warning("Got a transaction...") if $debug;

# Não faz nada se o criador não tiver sido o CAIS/RNP
my $requestorAddr = $ticket->RequestorAddresses;

if ( $requestorAddr =~ /reports\.spamcop\.net/ ){
  my $IP;
	$ticketParams{'incidentcategory'} = "spam";
	my $transactionObj = $ticket->Transactions;
	my $transaction = $transactionObj->Next;
	my $mensagem = $transaction->Content;
	my $spamcopRegex = /from [0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}/;
	if ( $textWithIp =~ $spamcopRegex ){
		$IP = $1;
		my $lenStr = length($1);
		$IP = substr($IP, 5, $lenStr);
	}
   # se $content estiver vazio, não faz nada
   if ($debug) {
       $RT::Logger->warning('$content está vazio');
   }

  if ( $content eq '' ) {
        my $message = <<"END_MESSAGE";
CONTENT VAZIO!

END_MESSAGE
        my $mime = MIME::Entity->build(
            From => 'CERT.Bahia <incidentes@pop-ba.rnp.br>',
            Data => $message
        );

        # notify internal team about the ticket to call the entity
        $ticket->Comment( MIMEObj => $mime );
        return 1;
    }

    # parser do arquivo iodef.xml e busca das informações
    my $parser = XMLin($content);


    $RT::Logger->warning("IP: $IP") if $debug;

    $ticketParams{'incidentcategory'} = "Copyright";

    # consulta ao PHPIpam para buscar informações do IP
    my $ua = LWP::UserAgent->new;
    $ua->timeout(10);
    $ua->default_header( 'User-Agent'   => 'PERL Client' );
    $ua->default_header( 'token'        => '0fe974b675f9cfc76e93fe48632522be' );
    $ua->default_header( 'Content-Type' => 'application/xml' );

    my $response = $ua->get(
"https://phpipam.intranet.pop-ba.rnp.br/api/certbahiaRT/subnets/overlapping/$IP/32/"
    );

    #PARSER DA INSTITUIÇÃO
    if ( $response->is_success ) {
        my $parser = XMLin( $response->decoded_content );

        if ( exists( $parser->{data}->{item2} ) ) {
            if ( $parser->{data}->{item0}->{description} =~
                /P2P-(\w+)-(\w+)\(?/ )
            {
                $ticketParams{'instituicao'} = $1;
                $ticketParams{'camp'}        = $2;
            }
            else {
                $ticketParams{'instituicao'} =
                  $parser->{data}->{item1}->{description};
                my @inst_name = $ticketParams{'instituicao'} =~ /.* (.+)$/;
                $ticketParams{'instituicao'} = $1;

                $ticketParams{'camp'} = $parser->{data}->{item0}->{description};
                my @camp_name = $ticketParams{'camp'} =~ /.*?[ -]?([^-]+)$/;
                $ticketParams{'camp'} = $2;

                if ( $ticketParams{'instituicao'} =~ /Clientes/ ) {
                    $ticketParams{'instituicao'} = $1;
                    $ticketParams{'camp'}        = "";
                }
            }
       }
        else {
            if ( $parser->{data}->{item0}->{description} =~ /.* - (.*) \(.*\)/ )
            {
                $ticketParams{'instituicao'} = $1;
            }
            elsif ( $parser->{data}->{item0}->{description} =~ /.* - (.*)/ ) {
                $ticketParams{'instituicao'} = $1;
            }
            else {
                $ticketParams{'instituicao'} =
                  $parser->{data}->{item0}->{description};
                my @inst_name = $ticketParams{'instituicao'} =~ /.* (.+)$/;
                $ticketParams{'instituicao'} = $inst_name[0];
            }

            if (   $ticketParams{'instituicao'} =~ /PTTBA-Site/
                or $ticketParams{'instituicao'} =~ /PoP-BA Outside/ )
            {
                $ticketParams{'instituicao'} = "PoP-BA";
            }
        }
    }

    $RT::Logger->warning("Inst: $ticketParams{'instituicao'}") if $debug;
    $RT::Logger->warning("Camp: $ticketParams{'camp'}")        if $debug;

    if ( $debug == 1)
    { # altera informações dos campos se tiver os dados necessários
       my $message = <<"END_MESSAGE";
Inst: $ticketParams{'instituicao'}
Camp: $ticketParams{'camp'}
Ip: $IP

END_MESSAGE
    my $mime = MIME::Entity->build(
        From => 'CERT.Bahia <incidentes@pop-ba.rnp.br>',
        Data => $message
    );
    }
if ($ticketParams{'incidentcategory'} ne '' and $ticketParams{'instituicao'} ne '') {
   $ticket->SetStatus("new");
   $ticket->AddCustomFieldValue(Field=>'NotificationSource', Value=>'Outros');
   $ticket->AddCustomFieldValue(Field=>'IncidentCategory', Value=>$ticketParams{'incidentcategory'});
   $ticket->AddCustomFieldValue(Field=>'Instituicao', Value=>$ticketParams{'instituicao'});

   if ($ticketParams{'camp'} ne '') {
      $ticket->AddCustomFieldValue(Field=>'InstituicaoCampus', Value=>$ticketParams{'camp'});
   }
        }
    # notify internal team about the ticket to call the entity
    return 1;
} else {
my $message = <<"END_MESSAGE";
FORA DA CONDIÇÃO

END_MESSAGE
        my $mime = MIME::Entity->build(
            From => 'CERT.Bahia <incidentes@pop-ba.rnp.br>',
            Data => $message
        );
# notify internal team about the ticket to call the entity
$ticket->Comment( MIMEObj => $mime );
return 0;
}
return 1;
1;
