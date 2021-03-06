using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Net.Sockets;
using System.Text;  
using System;
using UnityEngine.UI;
using TMPro;

public class controller : MonoBehaviour{


    private Socket s = null;
    private string ip;
    private int port;
    private int[] args;
    private char op;
    private int code;
    private int num_args;
    private float[] angles;
    private int acc =10000;

    private GameObject[] joints; //Joints
    private int[] inverted;
    // Start is called before the first frame update
    private GameObject tip;
    public TMP_Text  tip_location;
    public Text text_angles;
    
    /// <summary>
    /// Sets the localEulerAngles for each GameObject joint.
    /// </summary>
    public void set_angles(){
        for(int i=0;i<6;i++){
            GameObject joint = joints[i];
            if (i==0|| i==3||i == 5){
                joint.transform.localEulerAngles = new Vector3(0,inverted[i]*this.angles[i],0);

            }else if(i==2){
                    joint.transform.localEulerAngles = new Vector3(inverted[i]*this.angles[i]-90,0,0);
            }
            else{
                joint.transform.localEulerAngles  = new Vector3(inverted[i]*this.angles[i],0,0);

            }
        }
    }
    


    /// <summary>
    /// Gets the message stored in buff, (op,code,args)
    /// </summary>
    /// <param name="buf"></param>
    void read_message(byte[] buf){
        
        this.op = (char) buf[0];
        this.code = BitConverter.ToInt32(buf,1);
        this.num_args = BitConverter.ToInt32(buf,5); 
        for(int i=0;i< num_args ;i++){
            this.args[i] = BitConverter.ToInt32(buf,i*4+9);
        }
    }
    /// <summary>
    /// Transforms the values received in args and stores them in angles.
    /// </summary>
    public void update_angles(){
        for (int i =0;i<6;i++){
            this.angles[i] =(float) (((float)this.args[i]/(float)this.acc)/Math.PI)*(float)180.0;

        }
    }



    /// <summary>
    /// Called at the start of the prgram.
    /// </summary>
    void Start(){
        tip = GameObject.Find("tip");
        joints = new GameObject[6];
        angles = new float[6];
        inverted = new int[] {1,-1,-1,1,-1,1};
        for (int i =0;i<6;i++){
            joints[i] = GameObject.Find("J"+(i+1));
            
        }
       
        this.s = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        this.port = 65433;
        this.ip = "127.0.0.1";
        this.args = new Int32[7];
        try{
            this.s.Connect(this.ip, this.port);
        }catch{
            this.s = null;
        }
            

    }

    /// <summary>
    /// Called once per frame, receives data from the socket and reads the message to show the robot's angles.
    /// </summary>
    void Update(){
        if (this.s != null){
            this.s.Send(BitConverter.GetBytes(0)); 

            byte[] bytesRec = new Byte[128];
            int num_bytes = s.Receive(bytesRec,bytesRec.Length,0);  //Receiving response
            if (num_bytes == 38){
                this.read_message(bytesRec);
                this.update_angles();
                this.set_angles();
                this.s.Send(BitConverter.GetBytes(0)); 
            }
            int[] angles_round = new int[6];
            for(int i=0;i<6;i++){
                angles_round[i] = (int)Math.Round(this.angles[i])*inverted[i];
            }
            Vector3 pos = tip.transform.position;
            Vector3 ang = tip.transform.eulerAngles;
            this.tip_location.text = String.Format("X: {0:0.##} Y: {1:0.##} Z: {2:0.##}  A: {3:0.##} B: {4:0.##} C: {5:0.##}",pos[0],pos[1],pos[2],ang[0],ang[1],ang[2]);

            this.text_angles.text = "cur_angles ["+string.Join(" ,",angles_round)+ "]"; 
        }else{
             int[] angles_round = new int[6];
                for(int i=0;i<6;i++){
                    angles_round[i] = (int)Math.Round(this.angles[i]);
                } 
            text_angles.text = " ["+string.Join(" ,",angles_round)+ "]"; 


        }
       
                

    }
}
