import Errors from "../config/Errors.js";
import ClasterModel from "../Model/claster.model";
import CanvasClasterView from "../View/canvas.claster.view.js";

class ClasterController {
    private _clasterView: CanvasClasterView;
    private _clasterModel: ClasterModel;

    constructor(clasterModel: ClasterModel) {
        this._clasterModel = clasterModel;
        this._clasterView = new CanvasClasterView(this._clasterModel);
        this._clasterView.handleButtonClick(this.AddObjectCalback.bind(this));
        this._clasterView.handleDBSCANFetch(this.requestDBSCAN.bind(this));
    }
    
    AddObjectCalback(positionObject: {x: number, y: number}) {
        this._clasterModel.pushObject(positionObject.y, positionObject.x);
    }

    // hsv2rgb(h: number, s: number, v: number): string
    // {                              
    //   let f = (n, k =(n + h / 60) % 6) => v - v * s * Math.max(Math.min(k, 4 - k, 1), 0);     
    //   return `rgb(${f(5)}, ${f(3)}, ${f(1)})`;
    // } 

    requestDBSCAN(range: number, groupSize: number) {
        fetch(`alg/clasterisation?type=DBSCAN&range=${range}&gSize=${groupSize}`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/octet-stream'
            },

            body: (new Uint16Array(this._clasterModel.positions)).buffer
        }).then((responce) => {
            let reader = responce.body?.getReader();
            reader?.read().then(({ done, value }) => {
                console.log(value);

                if (value == undefined) Errors.handleError('undefined');
                
                let counter = 1;
                for (let i = 1; i < value.length; i += 2) {
                    if (value[i] != 1) {
                        if (value[i - 1] != counter) {
                            this._clasterView.changeCanvasView('', 'red', this._clasterModel.positions[i], this._clasterModel.positions[i - 1]);
                        }
                        else {
                            this._clasterView.changeCanvasView('', 'blue', this._clasterModel.positions[i], this._clasterModel.positions[i - 1]);
                        }
                    }
                    else {
                        this._clasterView.changeCanvasView('', 'grey', this._clasterModel.positions[i], this._clasterModel.positions[i - 1]);
                    }
                }
            })
        })
    }

    requestKMeans() {

    }
}

export default ClasterController;