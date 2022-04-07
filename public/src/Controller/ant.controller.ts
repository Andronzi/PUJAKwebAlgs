import AntView from "../View/ant.view";
import Controller from "./Controller.js";
import GraphModel from "../Model/graph.model";
import Errors from "../config/Errors.js";

class AntController extends Controller {
    private _graphView;
    private _graphModel;

    constructor(GraphView: AntView, GraphModel: GraphModel) {
        super();

        //classes Objects
        this._graphView = GraphView;
        this._graphModel = GraphModel;

        //set callbacks to view handlers
        this._graphView.setCoordsHandler(this.setCoords.bind(this));
        this._graphView.getDataHandler(this.getToken.bind(this));
    }

    //sendCoords data to model
    setCoords(x: number, y: number) {
        this._graphModel.pushObject(x, y);
    }

    //call calculate distances in model
    getToken(antsCount: number, greedCoef: number, herdCoef: number, pherLeak: number, pointsData: string) {
        // @ts-ignore
        let buff = buffer.Buffer.from(new Uint16Array(this._graphModel.coords).buffer);
        pointsData = buff.toString();

        let antData = {
            antsCount,
            greedCoef,
            herdCoef,
            pherLeak,
            pointsData
        }

        console.log(antData);

        this._graphModel.setDistances();

        fetch(`${this.urlValue}/alg/ants/launch`, {
            method: "POST",
            headers: {
                'Content-type': 'application/json'
            },
            body: JSON.stringify(antData)
        }).then((response) => {
            if (response.ok) {
                return response.text();
            }
        }).then((token) => {
            if (!sessionStorage.getItem('token')) {
                sessionStorage.setItem('token', token ?? Errors.handleError('undefined'));
            } else {
                fetch(`${this.urlValue}/alg/ants/terminateSession`, {
                    method: "GET",
                    headers: {
                        'Authorization': sessionStorage.getItem('token') ?? Errors.handleError('null')
                    }
                }).then(async() => {
                    console.log(sessionStorage.getItem('token'));
                    sessionStorage.removeItem('token');
                    await sessionStorage.setItem('token', token ?? Errors.handleError('undefined'));
                    console.log(sessionStorage.getItem('token'));
                }).catch((err) => {
                    console.log(err)
                })
            }
        }).then(async() => {
            await fetch(`${this.urlValue}/alg/ants/getState`, {
                method: "GET",
                headers: {
                    'Authorization': sessionStorage.getItem('token') ?? Errors.handleError('null'),
                },
            }).then((response) => {
                console.log(sessionStorage.getItem('token'))
                response.json().then(r => console.log(r));
            })
        })

    }

}

export default AntController;