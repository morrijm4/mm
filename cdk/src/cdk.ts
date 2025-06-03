import * as acm from 'aws-cdk-lib/aws-certificatemanager';
import * as cloudfront from 'aws-cdk-lib/aws-cloudfront';
import * as origins from 'aws-cdk-lib/aws-cloudfront-origins';
import * as ec2 from 'aws-cdk-lib/aws-ec2';
import { App, CfnOutput, Stack, type StackProps } from "aws-cdk-lib";
import type { Construct } from "constructs";

class WebServerStack extends Stack {
    constructor(scope: Construct, id: string, props?: StackProps) {
	super(scope, id, props);

	const vpc = ec2.Vpc.fromLookup(this, 'vpc', { isDefault: true });

	const keyPair = new ec2.KeyPair(this, 'ssh');

	const instance = new ec2.Instance(this, 'http', {
	    vpc,
	    keyPair,
	    allowAllOutbound: false,
	    instanceType: ec2.InstanceType.of(ec2.InstanceClass.T4G, ec2.InstanceSize.NANO),
	    machineImage: ec2.MachineImage.latestAmazonLinux2023({ cpuType: ec2.AmazonLinuxCpuType.ARM_64 }),
	    creditSpecification: ec2.CpuCredits.STANDARD,
	});

	new CfnOutput(this, 'public-ip', { value: instance.instancePublicIp });
	new CfnOutput(this, 'public-dns', { value: instance.instancePublicDnsName });

	instance.connections.allowFromAnyIpv4(ec2.Port.SSH, 'Allow ssh ingress');
	instance.connections.allowFromAnyIpv4(ec2.Port.tcp(8080), 'Allow dev http ingress');
	instance.connections.allowToAnyIpv4(ec2.Port.HTTPS, 'Allow all internet egress');

	new cloudfront.Distribution(this, 'cdn', {
	    certificate: acm.Certificate.fromCertificateArn(this, 'cert', 'arn:aws:acm:us-east-1:670799836323:certificate/45d4bc16-6650-4caf-b836-a3547282bdce'),
	    domainNames: ['test.mattymo.dev'],
	    defaultBehavior: {
		origin: new origins.HttpOrigin(instance.instancePublicDnsName, {
		    httpPort: 8080,
		    protocolPolicy: cloudfront.OriginProtocolPolicy.HTTP_ONLY,
		})
	    },
	});
    }
}

new WebServerStack(new App(), 'mm', {
    env: {
	region: 'us-east-1',
	account: '670799836323',
    }
});
